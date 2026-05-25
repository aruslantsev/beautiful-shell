#ifndef PROMPT_TERM_H
#define PROMPT_TERM_H

#include <unistd.h>
#include <sys/ioctl.h>
#include <cstddef>

int getTermParams(std::size_t &, std::size_t &);

#endif
