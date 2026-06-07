#pragma once
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <string>
#include <cstdlib>
#if defined(__linux__)
#include <sys/sysinfo.h>
#elif defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <mach/vm_statistics.h>
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined (__DragonFly__)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <vm/vm_param.h>
#include <unistd.h>
#include <cerrno>
#endif
#include "beautiful_shell.hpp"


class DateTimeModule : public BSModule {
private:
    std::string color_dark  = GREY;
    std::string color_light = BLACK;
public:
    std::string render(const BSContext &ctx, const BSSettings &cfg) const override {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        std::string active_color = (cfg.color_theme == color_theme::DARK) ? color_dark : color_light;
        return Colorizer::paint(oss.str(), active_color, ctx.shell, cfg.use_colors);
    }
};


class LoadAVGModule : public BSModule {
private:
    std::string color_dark  = GREY;
    std::string color_light = BLACK;
public:
    std::string render(const BSContext &ctx, const BSSettings &cfg) const override {
        double load[3];
        std::ostringstream out;
        out << "LoadAvg: ";
        if (getloadavg(load, 3) != -1) {
            out << std::fixed << std::setprecision(2) << load[0] << ", ";
            out << std::fixed << std::setprecision(2) << load[1] << ", ";
            out << std::fixed << std::setprecision(2) << load[2] << ".";
            std::string active_color = (cfg.color_theme == color_theme::DARK) ? color_dark : color_light;
            return Colorizer::paint(out.str(), active_color, ctx.shell, cfg.use_colors);
        }
        return "";
    }
};


class RAMModule : public BSModule {
private:
    std::string color_dark  = GREY;
    std::string color_light = BLACK;
public:
    std::string render(const BSContext &ctx, const BSSettings &cfg) const override {
        unsigned long long total_ram = 0;
        unsigned long long used_ram = 0;
        unsigned long long total_swap = 0;
        unsigned long long used_swap = 0;
        bool success = false;

#if defined(__linux__)
        struct sysinfo si;
        if (sysinfo(&si) == 0) {
            unsigned long long multiplier = si.mem_unit;
            total_ram = si.totalram * multiplier;
            used_ram = (si.totalram - si.freeram) * multiplier;
            total_swap = si.totalswap * multiplier;
            used_swap = (si.totalswap - si.freeswap) * multiplier;
            success = true;
        }
#elif defined(__APPLE__)
        uint64_t memsize = 0;
        size_t sz = sizeof(memsize);
        if (sysctlbyname("hw.memsize", &memsize, &sz, NULL, 0) == 0) {
            total_ram = memsize;

            vm_size_t page_size = 0;
            if (host_page_size(mach_host_self(), &page_size) == KERN_SUCCESS) {
                vm_statistics64_data_t vm;
                mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
                if (host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t)&vm, &count) == KERN_SUCCESS) {
                    unsigned long long free_pages = vm.free_count + vm.speculative_count;
                    used_ram = total_ram - (free_pages * page_size);

                    struct xsw_usage swap;
                    sz = sizeof(swap);
                    if (sysctlbyname("vm.swapusage", &swap, &sz, NULL, 0) == 0) {
                        total_swap = swap.xsu_total;
                        used_swap = swap.xsu_used;
                    }
                    success = true;
                }
            }
        }
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
        #if defined(__OpenBSD__) || defined(__NetBSD__)
            int mib[2] = {CTL_HW, HW_PHYSMEM64};
            uint64_t physmem = 0;
        #else
            int mib[2] = {CTL_HW, HW_PHYSMEM};
            unsigned long physmem = 0;
        #endif
        size_t sz = sizeof(physmem);
        
        if (sysctl(mib, 2, &physmem, &sz, NULL, 0) == 0) {
            total_ram = physmem;
            long page_size = sysconf(_SC_PAGESIZE);
            if (page_size > 0) {
                u_int free_pages = 0;
                sz = sizeof(free_pages);
                if (sysctlbyname("vm.stats.vm.v_free_count", &free_pages, &sz, NULL, 0) == 0) {
                    used_ram = total_ram - (static_cast<unsigned long long>(free_pages) * page_size);
                    success = true;
                } 
                #if defined(__OpenBSD__) || defined(__NetBSD__)
                else {
                    used_ram = total_ram;
                    success = true;
                }
                #endif

                #if defined(__FreeBSD__) || defined(__DragonFly__)
                int smib[2];
                size_t miblen = 2;
                if (sysctlnametomib("vm.swap_info", smib, &miblen) == 0) {
                    for (int n = 0; ; ++n) {
                        struct xswdev xsw;
                        int swap_mib[4];
                        size_t swap_miblen = miblen + 1;
                        
                        swap_mib[0] = smib[0];
                        swap_mib[1] = smib[1];
                        swap_mib[2] = n;
                        
                        sz = sizeof(xsw);
                        if (sysctl(swap_mib, swap_miblen, &xsw, &sz, NULL, 0) != 0) break;
                        
                        if (xsw.xsw_version == XSWDEV_VERSION) {
                            total_swap += static_cast<unsigned long long>(xsw.xsw_nblks) * page_size;
                            used_swap += static_cast<unsigned long long>(xsw.xsw_used) * page_size;
                        }
                    }
                }
                #endif
            }
        }
#endif

        if (!success) {
            return "";
        }

        std::ostringstream out;
        std::string unit;
        unsigned long long divisor = 1;
        if (total_ram <= 1024ULL * 1024 * 1024) {
            unit = "MB";
            divisor = 1024ULL * 1024;
        } else {
            unit = "GB";
            divisor = 1024ULL * 1024 * 1024;
        }
        
        out << "Mem: " << used_ram / divisor << "/" << total_ram / divisor << unit;
        if (total_swap > 0) {
            out << ", Swap: " << used_swap / divisor << "/" << total_swap / divisor << unit;
        }
        out << ".";

        std::string active_color = (cfg.color_theme == color_theme::DARK) ? color_dark : color_light;
        return Colorizer::paint(out.str(), active_color, ctx.shell, cfg.use_colors);
    }
};
