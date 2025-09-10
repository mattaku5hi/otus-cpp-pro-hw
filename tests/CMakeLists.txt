
if(ENABLE_BOOST_TEST)
    add_subdirectory(boost)
endif()

if(ENABLE_GOOGLE_TEST)
    add_subdirectory(gtest)
endif()