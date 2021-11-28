#pragma once

#define HTTPGD_VERSION "1.2.1.9000"

#include <boost/version.hpp>
#define HTTPGD_VERSION_BOOST BOOST_LIB_VERSION

#ifndef HTTPGD_NO_CAIRO
#include <cairo.h>
#define HTTPGD_VERSION_CAIRO CAIRO_VERSION_STRING
#else
#define HTTPGD_VERSION_CAIRO ""
#endif