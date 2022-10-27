#include "ugly.h"
#include "bad.h"

struct SplineImpl {
    int n;
    double a, b;
    SplineImpl(const std::vector<double>& xx, const std::vector<double>& yy, double a, double b)
        : a(a), b(b), x(xx), y(yy) {
        n = xx.size();
        y2.resize(n);
        mySplineSnd(&x[0], &y[0], n, a, b, &y2[0]);
    }
    double Interpolate(double xx) {
        double inval;
        mySplintCube(&x[0], &y[0], &y2[0], n, xx, &inval);
        return inval;
    }
    std::vector<double> x, y, y2;
};
Spline::Spline(const std::vector<double>& x, const std::vector<double>& y, double a, double b)
    : impl_(std::make_unique<SplineImpl>(x, y, a, b)) {
}

// Get spline value at a given point.
double Spline::Interpolate(double x) {
    return impl_->Interpolate(x);
}
