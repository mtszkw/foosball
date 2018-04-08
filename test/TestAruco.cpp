#include "catch.hpp"
#include "aruco.hpp"

// Nice, no double test macro in this lib?
#define REQUIRE_DOUBLE(a, b) REQUIRE(std::abs(a - b) <= 10e-8)

TEST_CASE( "Load configuration from default", "[aruco Configuration]" ) {
    cv::Ptr<cv::aruco::DetectorParameters> p = aruco::loadParametersFromFile("test/TestArucoConfig1.yaml");
    cv::Ptr<cv::aruco::DetectorParameters> d = cv::aruco::DetectorParameters::create();

    REQUIRE_DOUBLE(p->adaptiveThreshConstant, 8); // Changed value
    REQUIRE(p->adaptiveThreshWinSizeMax == d->adaptiveThreshWinSizeMax); // Changed, but wrong type
    REQUIRE(p->adaptiveThreshWinSizeMin == d->adaptiveThreshWinSizeMin); // Changed, but wrong type
    REQUIRE(p->cornerRefinementMaxIterations == d->cornerRefinementMaxIterations); // Not changed
    REQUIRE_DOUBLE(p->minMarkerPerimeterRate, d->minMarkerPerimeterRate); // In file, but not changed
    REQUIRE_DOUBLE(p->polygonalApproxAccuracyRate, 2); // Changed value
}