# Init the list of required boost libraries.
set(_PIRANHA_REQUIRED_BOOST_LIBS)

set(Boost_USE_STATIC_LIBS        OFF)  # only windows default is ON. Don't use ON cretes strange linking behaviour in Boost wanting unkown library z.lib

# Optional Boost serialization.
if(PIRANHA_WITH_BOOST_S11N)
	#list(APPEND _PIRANHA_REQUIRED_BOOST_LIBS serialization)
	set(PIRANHA_ENABLE_BOOST_S11N "#define PIRANHA_WITH_BOOST_S11N")
    find_package(Boost 1.76.0 REQUIRED COMPONENTS serialization)
endif()

# Optional Boost stacktrace.
if(PIRANHA_WITH_BOOST_STACKTRACE)
	set(PIRANHA_ENABLE_BOOST_STACKTRACE "#define PIRANHA_WITH_BOOST_STACKTRACE")
endif()

# Boost::iostreams is needed if any compression is enabled (in which case we need the iostreams filters).
if(PIRANHA_WITH_ZLIB OR PIRANHA_WITH_BZIP2)
	#list(APPEND _PIRANHA_REQUIRED_BOOST_LIBS iostreams) #inserts a ;. Unwanted
    find_package(Boost 1.76.0 REQUIRED COMPONENTS iostreams)
endif()

# Boost::python.
if(PIRANHA_BUILD_PYRANHA)
	if(${PYTHON_VERSION_MAJOR} EQUAL 2)
		list(APPEND _PIRANHA_REQUIRED_BOOST_LIBS python)
	else()
		list(APPEND _PIRANHA_REQUIRED_BOOST_LIBS python3)
	endif()
endif()

If(PIRANHA_BUILD_BENCHMARKS)
    #list(APPEND _PIRANHA_REQUIRED_BOOST_LIBS system filesystem)
    find_package(Boost 1.76.0 REQUIRED COMPONENTS system filesystem)
endif()

#replace all ";" with blanks
#string(REPLACE ";" " " _PIRANHA_REQUIRED_BOOST_LIBS "${_PIRANHA_REQUIRED_BOOST_LIBS}")

#message(STATUS "Required Boost libraries: ${_PIRANHA_REQUIRED_BOOST_LIBS}")

message(STATUS "Boost_USE_STATIC_LIBS   ${Boost_USE_STATIC_LIBS}")

#find_package(Boost 1.76.0 REQUIRED COMPONENTS "${_PIRANHA_REQUIRED_BOOST_LIBS}")

if(NOT Boost_FOUND)
	message(FATAL_ERROR "Not all the requested Boost components were found, exiting.")
endif()

message(STATUS "Detected Boost version: ${Boost_VERSION}")
message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")

# Cleanup.
unset(_PIRANHA_REQUIRED_BOOST_LIBS)
