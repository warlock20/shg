#include "shg/mstat.h"
#include <cmath>
#include <cstdlib>
#include <limits>
#include <vector>
#include "shg/mzt.h"
#include "testing.h"

namespace SHG::BTesting {

BOOST_AUTO_TEST_SUITE(mstat_test)

namespace {

// Cumulative distribution function of the Kolmogorov-Smirnov
// distribution as a sum of the series 1 - 2 \sum{j = 1}^{\infty}
// (-1)^{j - 1} exp(-2j^2z^2).
double F(const double x) {
     if (x <= 0.0)
          return 0.0;
     double s = 0.0;
     bool neg = false;
     for (int j = 1;; j++) {
          const double y = j * x;
          const double z = std::exp(-2.0 * y * y);
          if (z < std::numeric_limits<double>::min())
               break;
          if (neg) {
               s -= z;
               neg = false;
          } else {
               s += z;
               neg = true;
          }
     }
     s = 1.0 - 2.0 * s;
     if (s < 0.0)
          return 0.0;
     return s;
}

}  // anonymous namespace

namespace bdata = boost::unit_test::data;

BOOST_DATA_TEST_CASE(ksdist_test, bdata::xrange(501), xr) {
     const int i = xr;
     const double x = 0.01 * i;
     const double y = F(x);
     const double z = ksdist(x);
     const double d = std::abs(y - z);
     BOOST_CHECK(y >= 0.0);
     BOOST_CHECK(y <= 1.0);
     BOOST_CHECK(z >= 0.0);
     BOOST_CHECK(z <= 1.0);
     BOOST_CHECK(d < 1e-15);
}

BOOST_AUTO_TEST_CASE(chi2normtest_with_data_test) {
     const Vecdouble xd{0.57, 0.71, 0.85, 0.99, 1.13, 1.27, 1.41,
                        1.55, 1.69, 1.83, 1.97, 2.11, 2.28};
     const Vector<std::size_t> nd{1,  2,  9,  25, 37, 53, 56,
                                  53, 25, 19, 16, 3,  1};
     BOOST_CHECK(xd.size() == nd.size());
     Vecdouble x(sum(nd));
     for (std::size_t i = 0, n = 0; i < nd.size(); i++)
          for (std::size_t j = 0; j < nd[i]; j++)
               x(n++) = xd[i];
     const double p = chi2normtest(x, 10);
     BOOST_CHECK(std::abs(p) < 1e-12);
}

BOOST_AUTO_TEST_CASE(chi2normtest_generated_test) {
     const double mu = 5;
     const double sigma = std::sqrt(2.0);
     Vecdouble x(2000);
     MZT g;

     for (std::size_t i = 0; i < x.size(); i++)
          x(i) = mu + sigma * g.normal();
     const double p = chi2normtest(x, 10);
     BOOST_CHECK(std::abs(p - 0.889589) < 5e-7);
}

BOOST_AUTO_TEST_CASE(sample_test) {
     const Sample s(
          {1.0, 2.0, 2.0, 3.0, 3.0, 3.0, 4.0, 4.0, 4.0, 4.0});

     for (int i = 0; i <= 500; i++) {
          const double x = i / 100.0;
          const double F = s.cdf(x), F1 = s.lcdf(x);
          if (x < 1.0)
               BOOST_CHECK(std::abs(F - 0.0) < 1e-15);
          else if (x < 2.0)
               BOOST_CHECK(std::abs(F - 0.1) < 1e-15);
          else if (x < 3.0)
               BOOST_CHECK(std::abs(F - 0.3) < 1e-15);
          else if (x < 4.0)
               BOOST_CHECK(std::abs(F - 0.6) < 1e-15);
          else
               BOOST_CHECK(std::abs(F - 1.0) < 1e-15);
          if (x > 4.0)
               BOOST_CHECK(std::abs(F1 - 1.0) < 1e-15);
          else if (x > 3.0)
               BOOST_CHECK(std::abs(F1 - 0.6) < 1e-15);
          else if (x > 2.0)
               BOOST_CHECK(std::abs(F1 - 0.3) < 1e-15);
          else if (x > 1.0)
               BOOST_CHECK(std::abs(F1 - 0.1) < 1e-15);
          else
               BOOST_CHECK(std::abs(F1 - 0.0) < 1e-15);
     }

     for (int i = 1; i < 1000; i++) {
          const double p = i / 1000.0, q = s.quantile(p);
          if (p > 0.6)
               BOOST_CHECK(std::abs(q - 4.0) < 1e-15);
          else if (p > 0.3)
               BOOST_CHECK(std::abs(q - 3.0) < 1e-15);
          else if (p > 0.1)
               BOOST_CHECK(std::abs(q - 2.0) < 1e-15);
          else
               BOOST_CHECK(std::abs(q - 1.0) < 1e-15);
     }
}

/*
 * The results tested here are:
 *
 * 1.08037435341377e-02   9.73707791984890e-01
 * 6.52407729020162e-02   8.07194225353378e-08
 *
 * The results get with Numerical Recipes with (14.3.18) are:
 *
 * 1.08037435377046e-02   9.73012808686660e-01
 * 6.52407728998911e-02   7.35239129875459e-08
 */
BOOST_AUTO_TEST_CASE(ksnormtest_test) {
     MZT g;
     Vecdouble x(2000);
     double d, prob;

     for (std::size_t i = 0; i < x.size(); i++)
          x[i] = 3 + 0.5 * g.normal();
     ksnormtest(x, d, prob);
     BOOST_CHECK(std::abs(d - 1.08037435341377e-02) < 1e-6);
     BOOST_CHECK(std::abs(prob - 9.73707791984890e-01) < 1e-6);

     for (std::size_t i = 0; i < x.size(); i++)
          x[i] = g();
     ksnormtest(x, d, prob);
     BOOST_CHECK(std::abs(d - 6.52407729020162e-02) < 1e-6);
     BOOST_CHECK(std::abs(prob - 8.07194225353378e-08) < 1e-14);
}

BOOST_AUTO_TEST_CASE(run_length_distribution_test) {
     using std::vector;
     vector<int> x{0, 0, 1, 0, 2, 2, 1, 0, 1, 2,
                   2, 1, 0, 0, 0, 0, 1, 1, 2};
     vector<vector<int>> v = run_length_distribution(x, 3);
     const vector<vector<int>> r{
          {2, 1, 1, 4}, {1, 1, 1, 1, 2}, {2, 2, 1}};

     BOOST_CHECK(v.size() == r.size());
     for (std::size_t i = 0; i < v.size(); i++) {
          BOOST_CHECK(v[i].size() == r[i].size());
          for (std::size_t j = 0; j < v[i].size(); j++)
               BOOST_CHECK(v[i][j] == r[i][j]);
     }
     x[1] = 3;
     BOOST_CHECK_THROW(run_length_distribution(x, 3),
                       std::invalid_argument);
     x.resize(0);
     v = run_length_distribution(x, 3);
     BOOST_CHECK(v.size() == 3);
     for (std::size_t i = 0; i < v.size(); i++)
          BOOST_CHECK(v[i].size() == 0);
}

BOOST_AUTO_TEST_CASE(mle_lsd_test) {
     BOOST_CHECK(std::abs(mle_lsd(1.5) - 0.5335892440) < 1e-8);
     BOOST_CHECK(std::abs(mle_lsd(11.0) - 0.9762779470) < 1e-8);
     BOOST_CHECK(std::abs(mle_lsd(501.0) - 0.9997606870) < 3e-8);
     BOOST_CHECK_THROW(mle_lsd(0.9999), Invalid_argument);
}

BOOST_AUTO_TEST_CASE(cdf_lsd_test) {
     for (int k = 0; k < 100; k++)
          BOOST_CHECK(std::isfinite(cdf_lsd(k, 0.00001)));
     for (int k = 0; k < 100; k++)
          BOOST_CHECK(std::isfinite(cdf_lsd(k, 0.5)));
     for (int k = 0; k < 100; k++)
          BOOST_CHECK(std::isfinite(cdf_lsd(k, 0.99999)));
}

BOOST_AUTO_TEST_CASE(mle_negative_binomial_test) {
     const double t0 = 1.0;
     const double p0 = 0.5;
     MZT g;
     Vecint x(200);
     for (std::size_t i = 0; i < x.size(); i++)
          x(i) = g.negative_binomial(t0, p0);
     double t, p;
     mle_negative_binomial(x, t, p);
     BOOST_CHECK(std::abs(t - 1.05285074569) < 1e-9);
     BOOST_CHECK(std::abs(p - 0.50427490943) < 1e-9);
}

namespace {

// Calculate cdf of negative binomial distribution summing probability
// function.
double cdf_nb(double x, double t, double p) {
     const double q = 1.0 - p;
     BOOST_CHECK(t > 0.0 && p > 0.0 && q > 0.0);
     if (x < 0.0)
          return 0.0;
     const int n = ifloor(x);
     double b = 1.0, s = 1.0, z = 1.0;
     for (int i = 1; i <= n; i++) {
          b *= (t + i - 1) / i;
          z *= q;
          s += b * z;
     }
     return std::pow(p, t) * s;
}

}  // anonymous namespace

BOOST_AUTO_TEST_CASE(cdf_negative_binomial_test) {
     double t, p;
     auto test = [&t, &p](const double eps) {
          for (int i = -5; i <= 500; i++) {
               const double F0 = cdf_negative_binomial(i, t, p);
               const double F1 = cdf_nb(i, t, p);
               BOOST_CHECK(std::abs(F0 - F1) < eps);
          }
     };
     t = 1.000;
     p = 0.500;
     test(1e-9);
     t = 0.001;
     p = 0.999;
     test(1e-9);
     t = 0.001;
     p = 0.001;
     test(1e-7);
}

BOOST_AUTO_TEST_CASE(assessment_of_discrete_distribution_test) {
     const int n = 200;
     MZT g;
     Vecint x(n);
     // Sample from geometric distribution.
     {
          for (std::size_t i = 0; i < x.size(); i++)
               BOOST_CHECK((x(i) = g.geometric(0.5)) > 0);
          Assessment_of_discrete_distribution a(x);
          a.run();
          BOOST_CHECK(std::abs(a.geometric() - 1.0000000) < 5e-7);
          BOOST_CHECK(std::abs(a.poisson() - 0.0000000) < 5e-7);
          BOOST_CHECK(std::abs(a.logarithmic() - 0.5236627) < 5e-7);
          BOOST_CHECK(std::abs(a.negbin() - 0.0012100) < 5e-7);
     }
     // Sample from Poisson distribution.
     {
          for (std::size_t i = 0; i < x.size(); i++)
               BOOST_CHECK((x(i) = g.poisson(1.0)) >= 0);
          Assessment_of_discrete_distribution a(x);
          a.run();
          BOOST_CHECK(std::abs(a.geometric() - 0.0000000) < 5e-7);
          BOOST_CHECK(std::abs(a.poisson() - 0.6940636) < 5e-7);
          BOOST_CHECK(std::abs(a.logarithmic() - 0.0000000) < 5e-7);
          BOOST_CHECK(std::abs(a.negbin() - 1.0000000) < 5e-7);
     }
     // Sample from logarithmic series distribution.
     {
          for (std::size_t i = 0; i < x.size(); i++)
               BOOST_CHECK((x(i) = g.logarithmic(0.5)) > 0);
          Assessment_of_discrete_distribution a(x);
          a.run();
          BOOST_CHECK(std::abs(a.geometric() - 0.1649365) < 5e-7);
          BOOST_CHECK(std::abs(a.poisson() - 0.0000000) < 5e-7);
          BOOST_CHECK(std::abs(a.logarithmic() - 0.8575215) < 5e-7);
          BOOST_CHECK(std::abs(a.negbin() - 0.0000000) < 5e-7);
     }
     // Sample from negative binomial distribution.
     {
          for (std::size_t i = 0; i < x.size(); i++)
               BOOST_CHECK((x(i) = g.negative_binomial(1.0, 0.5)) >=
                           0);
          Assessment_of_discrete_distribution a(x);
          a.run();
          BOOST_CHECK(std::abs(a.geometric() - 0.0000000) < 5e-7);
          BOOST_CHECK(std::abs(a.poisson() - 0.0000379) < 5e-7);
          BOOST_CHECK(std::abs(a.logarithmic() - 0.0000000) < 5e-7);
          BOOST_CHECK(std::abs(a.negbin() - 0.9999419) < 5e-7);
     }
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace SHG::BTesting
