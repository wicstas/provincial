#include <pine/vecmath.h>

namespace pine {

// The following two functions are by Francesco Mazzoli
// detailed in article "Speeding up atan2f by 50x": https://mazzo.li/posts/vectorized-atan2.html
inline float atan_fma_approximation(float x) {
  auto a1 = 0.99997726f;
  auto a3 = -0.33262347f;
  auto a5 = 0.19354346f;
  auto a7 = -0.11643287f;
  auto a9 = 0.05265332f;
  auto a11 = -0.01172120f;
  auto x_sq = x * x;
  return x * fmaf(x_sq, fmaf(x_sq, fmaf(x_sq, fmaf(x_sq, fmaf(x_sq, a11, a9), a7), a5), a3), a1);
}
float atan2_approx(float y, float x) {
  float abs_y = std::fabs(y) + 1e-10f;
  float r = (x - std::copysign(abs_y, x)) / (abs_y + std::fabs(x));
  float angle = M_PI / 2.f - std::copysign(M_PI / 4.f, x);

  angle += (0.1963f * r * r - 0.9817f) * r;
  return std::copysign(angle, y);
}
// float atan2_approx(float y, float x) {
//   auto swap = fabs(x) < fabs(y);
//   auto atan_input = (swap ? x : y) / (swap ? y : x);
//   float res = atan_fma_approximation(atan_input);
//   res = swap ? copysignf(Pi2, atan_input) - res : res;
//   if (x < 0.0f) {
//     res = copysignf(Pi, y) + res;
//   }
//   return res;
// }

vec3 solve(mat3 m, vec3 b) {
  return inverse(m) * b;
  // {
  //   auto i = max_axis(abs(m[0]));
  //   if (i != 0) {
  //     psl::swap(m[0][0], m[0][i]);
  //     psl::swap(m[1][0], m[1][i]);
  //     psl::swap(m[2][0], m[2][i]);
  //     psl::swap(b[0], b[i]);
  //   }
  //   auto r1 = m[0][1] / m[0][0];
  //   auto r2 = m[0][2] / m[0][0];
  //   m[0][1] = 0;
  //   m[0][2] = 0;
  //   m[1][1] -= m[1][0] * r1;
  //   m[2][1] -= m[2][0] * r1;
  //   m[1][2] -= m[1][0] * r2;
  //   m[2][2] -= m[2][0] * r2;
  //   b[1] -= b[0] * r1;
  //   b[2] -= b[0] * r2;
  // }
  // {
  //   if (psl::abs(m[1][1]) < psl::abs(m[1][2])) {
  //     psl::swap(m[1][1], m[1][2]);
  //     psl::swap(m[2][1], m[2][2]);
  //     psl::swap(b[1], b[2]);
  //   }
  //   auto r2 = m[1][2] / m[1][1];
  //   m[1][2] = 0;
  //   m[2][2] -= m[2][1] * r2;
  //   b[2] -= b[1] * r2;
  // }
  // {
  //   b[1] -= b[2] * m[2][1] / m[2][2];
  //   b[0] -= b[2] * m[2][0] / m[2][2];
  //   b[0] -= b[1] * m[1][0] / m[1][1];
  // }

  // b[0] /= m[0][0];
  // b[1] /= m[1][1];
  // b[2] /= m[2][2];
  // return b;
}
mat3 inverse(const mat3& m) {
  mat3 r;

  float det = m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2]) +
              m[1][0] * (m[2][1] * m[0][2] - m[0][1] * m[2][2]) +
              m[2][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2]);
  if (det == 0)
    return r;

  r[0][0] = m[1][1] * m[2][2] - m[2][1] * m[1][2];
  r[0][1] = m[2][1] * m[0][2] - m[0][1] * m[2][2];
  r[0][2] = m[0][1] * m[1][2] - m[1][1] * m[0][2];

  r[1][0] = m[1][2] * m[2][0] - m[2][2] * m[1][0];
  r[1][1] = m[2][2] * m[0][0] - m[0][2] * m[2][0];
  r[1][2] = m[0][2] * m[1][0] - m[1][2] * m[0][0];

  r[2][0] = m[1][0] * m[2][1] - m[2][0] * m[1][1];
  r[2][1] = m[2][0] * m[0][1] - m[0][0] * m[2][1];
  r[2][2] = m[0][0] * m[1][1] - m[1][0] * m[0][1];

  return r / det;
}
mat4 inverse(const mat4& m) {
  mat4 r;
  float det = 0;
  for (int i = 0; i < 4; i++)
    det += (m[(1 + i) % 4][0] *
                (m[(2 + i) % 4][1] * m[(3 + i) % 4][2] - m[(3 + i) % 4][1] * m[(2 + i) % 4][2]) +
            m[(2 + i) % 4][0] *
                (m[(3 + i) % 4][1] * m[(1 + i) % 4][2] - m[(1 + i) % 4][1] * m[(3 + i) % 4][2]) +
            m[(3 + i) % 4][0] *
                (m[(1 + i) % 4][1] * m[(2 + i) % 4][2] - m[(2 + i) % 4][1] * m[(1 + i) % 4][2])) *
           m[i % 4][3] * (i % 2 ? -1 : 1);
  if (det == 0)
    return r;

  for (int v = 0; v < 4; v++)
    for (int i = 0; i < 4; i++)
      r[v][i] = (m[(1 + i) % 4][(1 + v) % 4] *
                     (m[(2 + i) % 4][(2 + v) % 4] * m[(3 + i) % 4][(3 + v) % 4] -
                      m[(3 + i) % 4][(2 + v) % 4] * m[(2 + i) % 4][(3 + v) % 4]) +
                 m[(2 + i) % 4][(1 + v) % 4] *
                     (m[(3 + i) % 4][(2 + v) % 4] * m[(1 + i) % 4][(3 + v) % 4] -
                      m[(1 + i) % 4][(2 + v) % 4] * m[(3 + i) % 4][(3 + v) % 4]) +
                 m[(3 + i) % 4][(1 + v) % 4] *
                     (m[(1 + i) % 4][(2 + v) % 4] * m[(2 + i) % 4][(3 + v) % 4] -
                      m[(2 + i) % 4][(2 + v) % 4] * m[(1 + i) % 4][(3 + v) % 4])) *
                ((v + i) % 2 ? 1 : -1);
  return r / det;
}

}  // namespace pine
