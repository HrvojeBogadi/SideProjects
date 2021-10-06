syms z s R0 R1 S0 S1 S2 x y

A = arx221.A;
B = arx221.B;
Ts = 0.01;

ta50 = 10*Ts;
zeta = 0.7;
wn = 2 / ta50;
alfa = 5;

% Uzmimo da je A+ = 1. Tada mozemo reci da je A_ = A
%
% Poremecaj se pojavljuje u obliku pritiska/usporenja rada motora
%   te ga mozemo modelirati kao STEP POREMECAJ

C = [1 -1];

degA_ = polynomialDegree(poly2sym(A));
degB_ = polynomialDegree(poly2sym(B));
degC = polynomialDegree(poly2sym(C));
degAp = 0;  % A+
degBp = 0;  % B+

degS_ = degA_ + degC - 1;
degR_ = degAp - degBp - degC + degS_;

degP = degA_ + degC + degR_;
degP_ = degA - degBp;
degPp = degP - degP_;


s = tf('s');

Pc_ = s^2 + 2*zeta*wn*s + wn^2;
Pcp = (s + alfa*zeta*wn)^2;

Pc_ = 1 / Pc_;
P_ = c2d(Pc_, Ts, 'matched');
[num, den] = tfdata(P_, 'v');
P_ = den;

Pcp = 1 / Pcp;
Pp = c2d(Pcp, Ts, 'matched');
[num, den] = tfdata(Pp, 'v');
Pp = den;

P = poly2sym(Pp, z) * poly2sym(P_, z);

Am = P_;
r = polynomialDegree(poly2sym(Am)) + degBp - degA_;
Bm_ = z^r;

bm0 = deconv(polyval(Am, 1), conv(polyval(sym2poly(Bm_), 1), polyval(B, 1)));

lhs = poly2sym(A, z) * poly2sym(C, z) * ( R1*z + R0 ) + poly2sym(B, z) * (S2 * z^2 + S1 * z + S0 );
rhs = P;

lhsCoeffs = coeffs(lhs, z, 'all');
rhsCoeffs = coeffs(rhs, z, 'all');

eq = rhsCoeffs == lhsCoeffs;
sol = solve(eq);
R0 = double(sol.R0);
R1 = double(sol.R1);
S0 = double(sol.S0);
S1 = double(sol.S1);
S2 = double(sol.S2);

R_ = [R1 R0];
S_ = [S2 S1 S0];

R = poly2sym(C, z) * poly2sym(R_, z);
S = poly2sym(S_, z);
T = poly2sym(bm0, z) * z^r * poly2sym(Pp, z);

vpa(expand(R));
vpa(expand(S));
vpa(expand(T));


R = sym2poly(R)
S = sym2poly(S)
T = sym2poly(T)

% Gx = tf(conv(B, T), (conv(A, R) + conv(B, S)), Ts)
Xr = tf(T, R, Ts);
Y = tf(S, R, Ts);

