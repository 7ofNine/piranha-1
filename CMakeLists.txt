cmake_minimum_required(VERSION 3.3.0)

project(piranha VERSION 0.11)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules" "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/yacma")

message(STATUS "System name: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Piranha version: ${piranha_VERSION}")

# Set default build type to "Release".
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING
		"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
	FORCE)
endif()

# Build option: enable test set.
option(PIRANHA_BUILD_TESTS "Build test set." OFF)

# Build option: enable benchmarks.
option(PIRANHA_BUILD_BENCHMARKS "Build benchmarks." OFF)

# Build option: build the Python bindings.
option(PIRANHA_BUILD_PYRANHA "Build Pyranha, the Python bindings for Piranha." OFF)

# Build option: enable the Boost serialization format.
option(PIRANHA_WITH_BOOST_S11N "Enable support for the Boost serialization format." OFF)

# Build option: enable the msgpack serialization format.
option(PIRANHA_WITH_MSGPACK "Enable support for the msgpack serialization format." OFF)

# Build option: enable zlib/gzip compression.
option(PIRANHA_WITH_ZLIB "Enable support for zlib/gzip compression." OFF)

# Build option: enable bzip2 compression.
option(PIRANHA_WITH_BZIP2 "Enable support for bzip2 compression." OFF)

# Build option: enable support for the Boost stacktrace library.
option(PIRANHA_WITH_BOOST_STACKTRACE "Enable support for the Boost stacktrace library." OFF)

# Build option: enable the installation of the library headers.
option(PIRANHA_INSTALL_HEADERS "Enable the installation of the library headers." ON)
mark_as_advanced(PIRANHA_INSTALL_HEADERS)

# A general-purpose option to signal that we intend to run Piranha under Valgrind.
# At the moment it just disables tests involving long double that give problems in Valgrind,
# in the future it might become a more general-purpose flag.
option(PIRANHA_RUN_ON_VALGRIND "Configure Piranha to be run on Valgrind." OFF)
# Make it an advanced option, not really interesting for non-developers.
mark_as_advanced(PIRANHA_RUN_ON_VALGRIND)

# Setup of the compilation flags.
include(YACMACompilerLinkerSettings)
include(CheckCXXCompilerFlag)

# Assemble the flags.
set(PIRANHA_CXX_FLAGS_DEBUG ${YACMA_CXX_FLAGS} ${YACMA_CXX_FLAGS_DEBUG} ${YACMA_THREADING_CXX_FLAGS})
set(PIRANHA_CXX_FLAGS_RELEASE ${YACMA_CXX_FLAGS} ${YACMA_THREADING_CXX_FLAGS})
set(PIRANHA_CXX_FLAGS_RELWITHDEBINFO ${YACMA_CXX_FLAGS} ${YACMA_CXX_FLAGS_RELWITHDEBINFO}${YACMA_THREADING_CXX_FLAGS}) 
# NOTE: the idea here is to enable globally certain compiler options that always need to be activated
# on a certain platform. Further compiler flags will be set for specific targets.
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND YACMA_COMPILER_IS_CLANGXX)
	message(STATUS "Clang compiler on OSX detected, setting the standard library to 'libc++'.")
	list(APPEND PIRANHA_CXX_FLAGS_DEBUG "-stdlib=libc++")
	list(APPEND PIRANHA_CXX_FLAGS_RELEASE "-stdlib=libc++")
endif()
if(YACMA_COMPILER_IS_MSVC)
	# Disable the idiotic minmax macros on MSVC and enable the bigobj option (but not with clang).
	list(APPEND PIRANHA_CXX_FLAGS_DEBUG "-DNOMINMAX")
    list(APPEND PIRANHA_CXX_FLAGS_DEBUG "-D_SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING")#We have to fix the allocator warnings
	list(APPEND PIRANHA_CXX_FLAGS_RELEASE "-DNOMINMAX")
    list(APPEND PIRANHA_CXX_FLAGS_RELEASE "-D_SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING")#We have to fix the allocator warnings
	list(APPEND PIRANHA_CXX_FLAGS_RELWITHDEBINFO "-DNOMINMAX")#We have to fix the allocator warnings
	if(YACMA_COMPILER_IS_CLANGXX)
		# clang-cl emits various warnings from the Boost/GMP/MPFR libraries, let's just silence them.
		# NOTE: at one point in the recent past, MSVC added an options similar to GCC's isystem:
		# https://blogs.msdn.microsoft.com/vcblog/2017/12/13/broken-warnings-theory/
		# We probably just need to wait for this to be picked up by CMake/clang-cl. Let's
		# revisit the issue in the future.
		list(APPEND _PIRANHA_CLANG_CL_DISABLED_WARNINGS
		"-Wno-unused-command-line-argument"
			"-Wno-unused-variable"
			"-Wno-inconsistent-dllimport"
			"-Wno-unknown-pragmas"
			"-Wno-unused-parameter"
			"-Wno-sign-compare"
			"-Wno-deprecated-declarations"
			"-Wno-deprecated-dynamic-exception-spec"
			"-Wno-old-style-cast"
			"-Wno-sign-conversion"
			"-Wno-non-virtual-dtor"
			"-Wno-deprecated"
			"-Wno-shadow"
			"-Wno-shorten-64-to-32"
			"-Wno-reserved-id-macro"
			"-Wno-undef"
			"-Wno-c++98-compat-pedantic"
			"-Wno-documentation-unknown-command"
			"-Wno-zero-as-null-pointer-constant"
			"-Wno-language-extension-token"
			"-Wno-gnu-anonymous-struct"
			"-Wno-nested-anon-types"
			"-Wno-documentation"
			"-Wno-comma"
			"-Wno-nonportable-system-include-path"
			"-Wno-global-constructors"
			"-Wno-redundant-parens"
			"-Wno-exit-time-destructors"
			"-Wno-missing-noreturn"
			"-Wno-switch-enum"
			"-Wno-covered-switch-default"
			"-Wno-float-equal"
			"-Wno-double-promotion"
			"-Wno-microsoft-enum-value"
			"-Wno-missing-prototypes"
			"-Wno-implicit-fallthrough"
			"-Wno-format-nonliteral"
			"-Wno-cast-qual"
			"-Wno-disabled-macro-expansion"
			"-Wno-unused-private-field"
			"-Wno-unused-template"
			"-Wno-unused-macros"
		)
		list(APPEND PIRANHA_CXX_FLAGS_DEBUG ${_PIRANHA_CLANG_CL_DISABLED_WARNINGS})
		list(APPEND PIRANHA_CXX_FLAGS_RELEASE ${_PIRANHA_CLANG_CL_DISABLED_WARNINGS})
		unset(_PIRANHA_CLANG_CL_DISABLED_WARNINGS)
	else()
		list(APPEND PIRANHA_CXX_FLAGS_DEBUG "/bigobj")
		list(APPEND PIRANHA_CXX_FLAGS_RELEASE "/bigobj")
	endif()
endif()
if(MINGW)
	# Flag needed to deal with big binaries in MinGW.
	list(APPEND PIRANHA_CXX_FLAGS_DEBUG "-Wa,-mbig-obj")
	list(APPEND PIRANHA_CXX_FLAGS_RELEASE "-Wa,-mbig-obj")
endif()

# Concepts detection.
if(YACMA_COMPILER_IS_GNUCXX)
    # This is just a hackish way of detecting concepts, need to revisit once
    # more compilers support them.
    check_cxx_compiler_flag("-fconcepts" PIRANHA_COMPILER_SUPPORTS_CONCEPTS)
endif()

# Threading setup.
include(YACMAThreadingSetup)
if(YACMA_HAVE_PTHREAD_AFFINITY)
	set(PIRANHA_PTHREAD_AFFINITY "#define PIRANHA_HAVE_PTHREAD_AFFINITY")
endif()

# Additional platform-specific setup.
include(PiranhaPlatformSettings)

# Setup the piranha interface library.
add_library(piranha INTERFACE)

# Start to set up the list of mandatory targets for piranha to link to.
target_link_libraries(piranha INTERFACE Threads::Threads)

# NOTE: ideally we would want this inside the pyranha CMakeLists.txt, however
# it seems like there's a strange interaction between the code for finding Boost.Python
# and the CMake FindPythonLibs macro, and it does not work that way.
if(PIRANHA_BUILD_PYRANHA)
	include(YACMAPythonSetup)
endif()

# Boost libraries setup.
include(PiranhaFindBoost)
target_link_libraries(piranha INTERFACE Boost::boost Boost::disable_autolinking)
if(PIRANHA_WITH_BOOST_S11N)
	target_link_libraries(piranha INTERFACE Boost::serialization)
endif()
if(PIRANHA_WITH_ZLIB OR PIRANHA_WITH_BZIP2)
	target_link_libraries(piranha INTERFACE Boost::iostreams)
endif()
# At the moment we hard-code the following config for the
# stacktrace library:
# - on Windows, we use windbg,
# - on Unix, we use libbacktrace.
if(PIRANHA_WITH_BOOST_STACKTRACE)
	if(WIN32)
		find_package(DbgEng REQUIRED)
		message(STATUS "DbgEng library found.")
		message(STATUS "DbgEng library is: ${DbgEng_LIBRARY}")
		target_link_libraries(piranha INTERFACE DbgEng::DbgEng)
	else()
		find_package(libbacktrace REQUIRED)
		message(STATUS "backtrace library found.")
		message(STATUS "backtrace include dir is: ${libbacktrace_INCLUDE_DIR}")
		message(STATUS "backtrace library is: ${libbacktrace_LIBRARY}")
		target_link_libraries(piranha INTERFACE libbacktrace::libbacktrace)
		if(UNIX AND NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
			# OSX apparently does not have libdl:
			# https://stackoverflow.com/questions/20169660/where-is-libdl-so-on-mac-os-x
			target_link_libraries(piranha INTERFACE ${CMAKE_DL_LIBS})
		endif()
	endif()
endif()

# Find mp++.
#set(PIRANHA_MINIMUM_MPPP_VERSION 0.9)
set(PIRANHA_MINIMUM_MPPP_VERSION 0.20)
find_package(mp++ ${PIRANHA_MINIMUM_MPPP_VERSION} REQUIRED)
target_link_libraries(piranha INTERFACE mp++::mp++)

if(PIRANHA_WITH_MSGPACK)
	find_package(MSGPACK-C REQUIRED)
	message(STATUS "msgpack-c library found.")
	message(STATUS "msgpack-c include dir is: ${MSGPACK-C_INCLUDE_DIR}")
	set(PIRANHA_ENABLE_MSGPACK "#define PIRANHA_WITH_MSGPACK")
	target_link_libraries(piranha INTERFACE MSGPACK-C::MSGPACK-C)
endif()

if(PIRANHA_WITH_ZLIB)
	find_package(ZLIB REQUIRED)
	message(STATUS "zlib library found.")
	message(STATUS "zlib include dir is: ${ZLIB_INCLUDE_DIR}")
	message(STATUS "zlib library is: ${ZLIB_LIBRARIES}")
	set(PIRANHA_ENABLE_ZLIB "#define PIRANHA_WITH_ZLIB")
	target_link_libraries(piranha INTERFACE ZLIB::ZLIB)
endif()

if(PIRANHA_WITH_BZIP2)
	include(PiranhaFindBZip2)
	message(STATUS "bzip2 library found.")
	message(STATUS "bzip2 include dir is: ${BZIP2_INCLUDE_DIR}")
	message(STATUS "bzip2 library is: ${BZIP2_LIBRARIES}")
	set(PIRANHA_ENABLE_BZIP2 "#define PIRANHA_WITH_BZIP2")
	target_link_libraries(piranha INTERFACE BZip2::BZip2)
endif()

# Finish setting up the piranha interface library.
target_include_directories(piranha INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
	$<INSTALL_INTERFACE:include>)

# Try to determine the git revision.
find_package(Git)
if(Git_FOUND)
	message(STATUS "Git executable: ${GIT_EXECUTABLE}")
	execute_process(COMMAND ${GIT_EXECUTABLE} "log" "--no-color" "-n1" "--date=short" "--pretty=format:%H" WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} OUTPUT_VARIABLE PIRANHA_GIT_REVISION OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
endif()
if(NOT PIRANHA_GIT_REVISION)
	set(PIRANHA_GIT_REVISION "unknown")
endif()
message(STATUS "Git revision: ${PIRANHA_GIT_REVISION}")

# Configure config.hpp.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in" "${CMAKE_CURRENT_BINARY_DIR}/include/piranha/config.hpp" @ONLY)

# Configure the doc config file.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/doc/conf.py.in" "${CMAKE_CURRENT_SOURCE_DIR}/doc/conf.py" @ONLY)

# Installation.
if(PIRANHA_INSTALL_HEADERS)
	# Setup of the export.
	install(TARGETS piranha EXPORT piranha_export)
	set(_PIRANHA_CONFIG_OPTIONAL_DEPS)
	if(PIRANHA_WITH_ZLIB)
		set(_PIRANHA_CONFIG_OPTIONAL_DEPS "${_PIRANHA_CONFIG_OPTIONAL_DEPS}find_package(ZLIB REQUIRED)\nset(PIRANHA_WITH_ZLIB TRUE)\n")
	endif()
	if(PIRANHA_WITH_BZIP2)
		set(_PIRANHA_CONFIG_OPTIONAL_DEPS "${_PIRANHA_CONFIG_OPTIONAL_DEPS}include(PiranhaFindBZip2)\nset(PIRANHA_WITH_BZIP2 TRUE)\n")
		install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/PiranhaFindBZip2.cmake" DESTINATION "lib/cmake/piranha")
	endif()
	if(PIRANHA_WITH_BOOST_S11N)
		set(_PIRANHA_CONFIG_OPTIONAL_DEPS "${_PIRANHA_CONFIG_OPTIONAL_DEPS}set(PIRANHA_WITH_BOOST_S11N TRUE)\n")
	endif()
	if(PIRANHA_WITH_BOOST_STACKTRACE)
		set(_PIRANHA_CONFIG_OPTIONAL_DEPS "${_PIRANHA_CONFIG_OPTIONAL_DEPS}set(PIRANHA_WITH_BOOST_STACKTRACE TRUE)\n")
		if(WIN32)
			install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/FindDbgEng.cmake" DESTINATION "lib/cmake/piranha")
		else()
			install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/Findlibbacktrace.cmake" DESTINATION "lib/cmake/piranha")
		endif()
	endif()
	if(PIRANHA_WITH_MSGPACK)
		set(_PIRANHA_CONFIG_OPTIONAL_DEPS "${_PIRANHA_CONFIG_OPTIONAL_DEPS}find_package(MSGPACK-C REQUIRED)\nset(PIRANHA_WITH_MSGPACK TRUE)\n")
		install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/FindMSGPACK-C.cmake" DESTINATION "lib/cmake/piranha")
	endif()
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/piranha-config.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/piranha-config.cmake" @ONLY)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/piranha-config.cmake" DESTINATION "lib/cmake/piranha")
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/PiranhaFindBoost.cmake" DESTINATION "lib/cmake/piranha")
	install(EXPORT piranha_export NAMESPACE piranha:: DESTINATION lib/cmake/piranha)
	# Take care of versioning.
	include(CMakePackageConfigHelpers)
	# NOTE: for the moment, set COMPATIBILITY to ExactVersion, as piranha is evolving fast.
	write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/piranha-config-version.cmake" VERSION ${piranha_VERSION}
		COMPATIBILITY ExactVersion)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/piranha-config-version.cmake" DESTINATION "lib/cmake/piranha")

	# Installation of the header files.
	install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/piranha" DESTINATION include)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/piranha/config.hpp" DESTINATION include/piranha)
endif()

# Tests and benchmarks setup.
# This is just a simple counter variable, internal use only.
set(_PIRANHA_TEST_NUM "0")
# Check splitting options. These need to be set from the command line.
# - PIRANHA_TEST_NSPLIT: number of chunks into which the unit tests will be divided (must be > 1).
# - PIRANHA_TEST_SPLIT_NUM: 0-based index of the chunk to run.
if(PIRANHA_TEST_NSPLIT AND "${PIRANHA_TEST_SPLIT_NUM}" STREQUAL "")
	message(FATAL_ERROR "Test splitting was requested, but the PIRANHA_TEST_SPLIT_NUM variable was not set.")
elseif(NOT PIRANHA_TEST_NSPLIT AND NOT "${PIRANHA_TEST_SPLIT_NUM}" STREQUAL "")
	message(FATAL_ERROR "The PIRANHA_TEST_SPLIT_NUM variable was set, but test splitting was not requested.")
endif()
if(PIRANHA_TEST_NSPLIT)
	message(STATUS "Tests will be split into ${PIRANHA_TEST_NSPLIT} chunks. The chunk with index ${PIRANHA_TEST_SPLIT_NUM} will be processed.")
endif()

if(PIRANHA_BUILD_TESTS)
	enable_testing()
	add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tests")
endif()

if(PIRANHA_BUILD_BENCHMARKS)
	add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/benchmarks")
endif()

IF(PIRANHA_BUILD_PYRANHA)
	add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/pyranha")
	if(MINGW)
		message(STATUS "Creating the files for the generation of a binary wheel for MinGW.")
		configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tools/mingw_wheel_setup.py" "${CMAKE_CURRENT_BINARY_DIR}/wheel/setup.py" @ONLY)
		configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tools/mingw_wheel_libs_python${PYTHON_VERSION_MAJOR}.txt" "${CMAKE_CURRENT_BINARY_DIR}/wheel/mingw_wheel_libs_python${PYTHON_VERSION_MAJOR}.txt" @ONLY)
	endif()
endif()

# Target to get the header files into MSVC	
SET(INCS
    include/piranha/array_key.hpp
    include/piranha/
    include/piranha/base_series_multiplier.hpp
    include/piranha/cache_aligning_allocator.hpp
    include/piranha/convert_to.hpp
    include/piranha/divisor.hpp
    include/piranha/divisor_series.hpp
    include/piranha/dynamic_aligning_allocator.hpp
    include/piranha/exceptions.hpp
    include/piranha/forwarding.hpp
    include/piranha/hash_set.hpp
    include/piranha/integer.hpp
    include/piranha/invert.hpp
    include/piranha/ipow_substitutable_series.hpp
    include/piranha/is_cf.hpp
    include/piranha/is_key.hpp
    include/piranha/key_is_convertible.hpp
    include/piranha/key_is_multipliable.hpp
    include/piranha/kronecker_array.hpp
    include/piranha/kronecker_monomial.hpp
    include/piranha/lambdify.hpp
    include/piranha/math.hpp
    include/piranha/memory.hpp
    include/piranha/monomial.hpp
    include/piranha/piranha.hpp
    include/piranha/poisson_series.hpp
    include/piranha/polynomial.hpp
    include/piranha/power_series.hpp
    include/piranha/print_tex_coefficient.hpp
    include/piranha/rational.hpp
    include/piranha/real.hpp
    include/piranha/real_trigonometric_kronecker_monomial.hpp
    include/piranha/runtime_info.hpp
    include/piranha/s11n.hpp
    include/piranha/safe_cast.hpp
    include/piranha/safe_convert.hpp
    include/piranha/series.hpp
    include/piranha/series_multiplier.hpp
    include/piranha/settings.hpp
    include/piranha/small_vector.hpp
    include/piranha/static_vector.hpp
    include/piranha/symbol_utils.hpp
    include/piranha/t_substitutable_series.hpp
    include/piranha/term.hpp
    include/piranha/thread_barrier.hpp
    include/piranha/thread_management.hpp
    include/piranha/thread_pool.hpp
    include/piranha/trigonometric_series.hpp
    include/piranha/tuning.hpp
    include/piranha/type_traits.hpp
    include/piranha/math/binomial.hpp
    include/piranha/math/cos.hpp
    include/piranha/math/degree.hpp
    include/piranha/math/gcd.hpp
    include/piranha/math/gcd3.hpp
    include/piranha/math/is_one.hpp
    include/piranha/math/is_zero.hpp
    include/piranha/math/ldegree.hpp
    include/piranha/math/pow.hpp
    include/piranha/math/sin.hpp
    include/piranha/key/key_degree.hpp
    include/piranha/key/key_is_one.hpp
    include/piranha/key/key_is_zero.hpp
    include/piranha/key/key_ldegree.hpp
    include/piranha/detail/atomic_flag_array.hpp
    include/piranha/detail/atomic_lock_guard.hpp
    include/piranha/detail/cf_mult_impl.hpp
    include/piranha/detail/config_clang.hpp
    include/piranha/detail/config_gcc.hpp
    include/piranha/detail/config_intel.hpp
    include/piranha/detail/debug_access.hpp
    include/piranha/detail/demangle.hpp
    include/piranha/detail/divisor_series_fwd.hpp
    include/piranha/detail/init.hpp
    include/piranha/detail/km_commons.hpp
    include/piranha/detail/monomial_common.hpp
    include/piranha/detail/parallel_vector_transform.hpp
    include/piranha/detail/poisson_series_fwd.hpp
    include/piranha/detail/polynomial_fwd.hpp
    include/piranha/detail/prepare_for_print.hpp
    include/piranha/detail/safe_integral_arith.hpp
    include/piranha/detail/series_fwd.hpp
    include/piranha/detail/series_multiplier_fwd.hpp
    include/piranha/detail/sfinae_types.hpp
    include/piranha/detail/small_vector_fwd.hpp
    include/piranha/detail/stacktrace.hpp
    include/piranha/detail/vector_hasher.hpp
    tests/exception_matcher.hpp
)
SOURCE_GROUP(TREE ${CMAKE_SOURCE_DIR} PREFIX "Header Files" FILES ${INCS})
ADD_LIBRARY(Headers STATIC ${INCS})
SET_TARGET_PROPERTIES(Headers PROPERTIES LINKER_LANGUAGE CXX)
TARGET_INCLUDE_DIRECTORIES(Headers PRIVATE include/piranha/ include/piranha/detail/ include/piranha/math/ include/piranha/key/)
