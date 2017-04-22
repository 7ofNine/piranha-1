set(_PIRANHA_REQUIRED_BOOST_LIBS serialization iostreams)
if(PIRANHA_BUILD_TESTS OR PIRANHA_BUILD_BENCHMARKS)
	# These libraries are needed only if building tests.
	list(APPEND _PIRANHA_REQUIRED_BOOST_LIBS filesystem system)
endif()
if(PIRANHA_BUILD_PYRANHA)
	list(APPEND _PIRANHA_REQUIRED_BOOST_LIBS python)
endif()

message(STATUS "Required Boost libraries: ${_PIRANHA_REQUIRED_BOOST_LIBS}")

find_package(Boost 1.58.0 REQUIRED COMPONENTS "${_PIRANHA_REQUIRED_BOOST_LIBS}")

if(NOT Boost_FOUND)
	message(FATAL_ERROR "Not all requested Boost components were found, exiting.")
endif()

message(STATUS "Detected Boost version: ${Boost_VERSION}")
message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")

# Might need to recreate these targets if they are missing (e.g., older CMake versions).
if(NOT TARGET Boost::boost)
    message(STATUS "The 'Boost::boost' target is missing, creating it.")
    add_library(Boost::boost INTERFACE IMPORTED)
    set_target_properties(Boost::boost PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIRS}")
endif()

if(NOT TARGET Boost::disable_autolinking)
    message(STATUS "The 'Boost::disable_autolinking' target is missing, creating it.")
    add_library(Boost::disable_autolinking INTERFACE IMPORTED)
    if(WIN32)
        set_target_properties(Boost::disable_autolinking PROPERTIES INTERFACE_COMPILE_DEFINITIONS "BOOST_ALL_NO_LIB")
    endif()
endif()

if(NOT TARGET Boost::serialization)
	# NOTE: CMake's Boost finding module will not provide imported targets for recent Boost versions, as it needs
	# an explicit mapping specifying the dependencies between the various Boost libs (and this is version-dependent).
	# If we are here, it means that Boost was correctly found with all the needed components, but the Boost version
	# found is too recent and imported targets are not available. We will reconstruct them here in order to be able
	# to link to targets rather than using the variables defined by the FindBoost.cmake module.
	# NOTE: in Piranha's case, we are lucky because all the Boost libs we need don't have any interdependency
	# with other boost libs.
	message(STATUS "The imported Boost targets are not available, creating them manually.")
	foreach(_PIRANHA_BOOST_COMPONENT ${_PIRANHA_REQUIRED_BOOST_LIBS})
		message(STATUS "Creating the 'Boost::${_PIRANHA_BOOST_COMPONENT}' imported target.")
		string(TOUPPER ${_PIRANHA_BOOST_COMPONENT} _PIRANHA_BOOST_UPPER_COMPONENT)
		if(Boost_USE_STATIC_LIBS)
			add_library(Boost::${_PIRANHA_BOOST_COMPONENT} STATIC IMPORTED)
		else()
			add_library(Boost::${_PIRANHA_BOOST_COMPONENT} UNKNOWN IMPORTED)
		endif()
		set_target_properties(Boost::${_PIRANHA_BOOST_COMPONENT} PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIRS}")
		set_target_properties(Boost::${_PIRANHA_BOOST_COMPONENT} PROPERTIES
			IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
			IMPORTED_LOCATION "${Boost_${_PIRANHA_BOOST_UPPER_COMPONENT}_LIBRARY}")
	endforeach()
	# NOTE: the FindBoost macro also sets the Release and Debug counterparts of the properties above.
	# It seems like it is not necessary for our own uses, but keep it in mind for the future.
else()
	# FindBoost.cmake marks regexp as a dependency for iostreams, but it is true only if one uses the iostreams
	# regexp filter (we don't). Remove the dependency.
	message(STATUS "Removing the dependency of 'Boost::iostreams' on 'Boost::regex'.")
	get_target_property(_PIRANHA_BOOST_IOSTREAMS_LINK_LIBRARIES Boost::iostreams INTERFACE_LINK_LIBRARIES)
	list(REMOVE_ITEM _PIRANHA_BOOST_IOSTREAMS_LINK_LIBRARIES "Boost::regex")
	set_target_properties(Boost::iostreams PROPERTIES INTERFACE_LINK_LIBRARIES "${_PIRANHA_BOOST_IOSTREAMS_LINK_LIBRARIES}")
endif()
