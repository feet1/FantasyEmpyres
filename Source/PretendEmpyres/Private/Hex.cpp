#include "PretendEmpyres.h"
#include "Hex.h"
// Hex grid coordinate mapping, center is 0,0,0 coordinate for Q,R,S
// negative values are expressed here as their Shift-key value (ie  -4 is a $)
//                    ___
// Coordinate setup: /Q R\
//                   \_S_/
//
//
//                                 +R North
//                                  .
//                                  .               
//                                  .               
//     -S                  ___     ___     ___                 +Q NorthEast
//          .          ___/@ 4\___/0 3\___/2 2\___          .
//              .     /# 4\_@_/! 3\_#_/1 2\_$_/3 1\     .
//                  . \_!_/@ 3\_@_/0 2\_#_/2 1\_$_/ .                                                     
//                    /#.3\_!_/! 2\_@_/1 1\_#_/3.0\
//                    \_0_/@.2\_!_/0 1\_@_/2.0\_#_/  
//                    /# 2\_0_/!.1\_!_/1.0\_@_/3 !\
//                    \_1_/@ 1\_0_/Q.R\_!_/2 !\_@_/  
//                    /# 1\_1_/!.0\_S_/1.!\_!_/3 @\
//                    \_2_/@.0\_1_/0 !\_0_/2.@\_!_/
//                    /#.0\_2_/! !\_1_/1 @\_0_/3.#\
//                  . \_3_/@ !\_2_/0 @\_1_/2 #\_0_/ .
//              .     /# !\_3_/! @\_2_/1 #\_1_/3 $\     .
//          .         \_4_/@ @\_3_/0 #\_2_/2 $\_1_/         .
//     -Q                 \_4_/   \_3_/   \_2_/                +S SouthEast
//                                  .              
//                                  .              
//                                  .
//                                 -R

const Hexagon Hexagon::NeighborVectors[SidesOnAHex] = { 
        Hexagon( 0,  1, -1) // N  -> +R (Y axis)
      , Hexagon( 1,  0, -1) // NE -> +Q (X axis with + slope)
      , Hexagon( 1, -1,  0) // SE -> +S (X axis with - slope)
      , Hexagon( 0, -1,  1) // S  -> -R
      , Hexagon(-1,  0,  1) // SW -> -Q
      , Hexagon(-1,  1,  0) // NW -> -S
};

Hexagon::NeighborAccessor::NeighborAccessor(const Hexagon * const Parent) 
   : parent(Parent) 
{

}

Hexagon Hexagon::NeighborAccessor::operator[] (const int index) const
{
   checkf(index >= 0 && index < Hexagon::CoordCount, TEXT("Hexagon::NeighborAccessor[] index out of range (%d)"), index);
   return *parent + NeighborVectors[index];
}

Hexagon::Hexagon()
   :q(0), r(0), s(0), w(0), neighbors(this)
{

}

Hexagon::Hexagon(int Q, int R, int S, int W)
   : q(Q), r(R), s(S), w(W), neighbors(this)
{

}

Hexagon::Hexagon(const Hexagon& other)
   : q(other.q), r(other.r), s(other.s), w(other.w), neighbors(this)
{

}

Hexagon::Hexagon(const HexagonFraction& other)
   : q((int32)other.q), r((int32)other.r), s((int32)other.s), w((int32)other.w), neighbors(this)
{

}

bool Hexagon::operator==(const Hexagon& rhs) const
{
   return q == rhs.q && r == rhs.r && s == rhs.s && w == rhs.w;
}

bool Hexagon::operator!=(const Hexagon& rhs) const
{
   return q != rhs.q || r != rhs.r || s != rhs.s || w != rhs.w;
}

Hexagon& Hexagon::operator=(const Hexagon& rhs)
{
   q = rhs.q;
   r = rhs.r;
   s = rhs.s;
   w = rhs.w;
   return *this;
}

int32& Hexagon::operator[](const int index)
{
   checkf(index >= 0 && index < Hexagon::CoordCount, TEXT("Hexagon::operator[] (non-const) was given an out of range index (%d)."), index);
   return v[index];
}

const int32& Hexagon::operator[](const int index) const
{
   checkf(index >= 0 && index < Hexagon::CoordCount, TEXT("Hexagon::operator[] (const) was given an out of range index (%d)."), index);
   return v[index];
}

Hexagon Hexagon::operator-() const
{
   return Hexagon(-q, -r, -s, w); //w is not negated, it is either 0 or 1
}

Hexagon Hexagon::operator+(const Hexagon& rhs) const
{
   //w0 means it's a direction (vector), w1 means its a point (vertex)
   // two points can not be added together logically (LA + Chicago = ?)
   int W = r + rhs.w;
   checkf(W == 0 || W == 1, TEXT("Hexagon::operator+ Two 'points' (w=1) can not be added together."));
   return Hexagon(q + rhs.q, r + rhs.r, s + rhs.s, W);
}

Hexagon Hexagon::operator-(const Hexagon& rhs) const
{
   //w0 means it's a direction (vector), w1 means its a point (vertex)
   // a point can not be subtracted from a vector ( 5 miles north minus Chicago = ?)
   int W = r - rhs.w;
   checkf(W == 0 || W == 1, TEXT("Hexagon::operator- A 'point' (w=1) can not be subtracted from a vector (w=0). Did you mean to do this operation in reverse (point - vector)?"));
   return Hexagon(q - rhs.q, r - rhs.r, s - rhs.s, W);
}

Hexagon Hexagon::operator*(const float scalar) const
{
   checkf(w == 0, TEXT("Hexagon::operator* Scaling a point (w=1) doesn't make sense. Did you mean to use a vector (w=0)?"));
   return Hexagon((int)(q * scalar), (int)(r * scalar), (int)(s * scalar), w);
}

Hexagon Hexagon::operator*(const int scalar) const
{
   checkf(w == 0, TEXT("Hexagon::operator* Scaling a point (w=1) doesn't make sense. Did you mean to use a vector (w=0)?"));
   return Hexagon(q * scalar, r * scalar, s * scalar, w);
}

Hexagon Hexagon::operator%(const int mod) const
{
   checkf(mod != 0, TEXT("Hexagon::operator% division by zero."));
   return Hexagon(q % mod, r % mod, s % mod, w);
}

//Friend function
Hexagon operator*(const float& scalar, const Hexagon& hex)
{
   return hex * scalar;
}

//Friend function
Hexagon operator*(const int& scalar, const Hexagon& hex)
{
   return hex * scalar;
}

Hexagon& Hexagon::operator+=(const Hexagon& rhs)
{
   //w0 means it's a direction (vector), w1 means its a point (vertex)
   // two points can not be added together logically (LA + Chicago = ?)
   int W = r + rhs.w;
   checkf(W == 0 || W == 1, TEXT("Hexagon::operator+= Two 'points' (w=1) can not be added together."));
   q += rhs.q;
   r += rhs.r;
   s += rhs.s;
   w = W;
   return *this;
}

Hexagon& Hexagon::operator-=(const Hexagon& rhs)
{
   //w0 means it's a direction (vector), w1 means its a point (vertex)
   // a point can not be subtracted from a vector ( 5 miles north minus Chicago = ?)
   int W = r - rhs.w;
   checkf(W == 0 || W == 1, TEXT("Hexagon::operator-= A 'point' (w=1) can not be subtracted from a vector (w=0). Did you mean to do this operation in reverse (point - vector)?"));
   q -= rhs.q;
   r -= rhs.r;
   s -= rhs.s;
   w = W;
   return *this;
}

Hexagon& Hexagon::operator*=(const float scalar)
{
   checkf(w == 0, TEXT("Hexagon::operator*= Scaling a point (w=1) doesn't make sense. Did you mean to use a vector (w=0)?"));
   q = (int)(q * scalar);
   r = (int)(r * scalar);
   s = (int)(s * scalar);
   return *this;
}

Hexagon& Hexagon::operator*=(const int scalar)
{
   checkf(w == 0, TEXT("Hexagon::operator*= Scaling a point (w=1) doesn't make sense. Did you mean to use a vector (w=0)?"));
   q *= scalar;
   r *= scalar;
   s *= scalar;
   return *this;
}

Hexagon& Hexagon::operator%=(const int mod)
{
   checkf(mod != 0, TEXT("Hexagon::operator%= division by zero."));
   q %= mod;
   r %= mod;
   s %= mod;
   return *this;
}

int Hexagon::Magnatude() const
{
   checkf(w == 0, TEXT("Hexagon::Magnatude doesn't make sense on a point (w=1). Did you mean to use a vector (w=0)?"));
   int mag = (FMath::Abs(q) + FMath::Abs(r) + FMath::Abs(s)) / 2;
   return mag;
}

int Hexagon::DistanceTo(const Hexagon& rhs) const
{
   checkf(w == 1 && rhs.w == 1, TEXT("Hexagon::DistanceTo doesn't make sense with vectors (w=0). Did you mean to use points (w=1)?"));
   int distance = (FMath::Abs(rhs.q - q) + FMath::Abs(rhs.r - r) + FMath::Abs(rhs.s - s)) / 2;
   return distance;
}

bool Hexagon::IsPoint() const
{
   return w == 1;
}

bool Hexagon::IsVector() const
{
   return w == 0;
}

const HexagonF HexagonF::NeighborVectors[SidesOnAHex] = {
   HexagonF(0.f,  1.f, -1.f) // N  -> +R (Y axis)
   , HexagonF(1.f,  0.f, -1.f) // NE -> +Q (X axis with + slope)
   , HexagonF(1.f, -1.f,  0.f) // SE -> +S (X axis with - slope)
   , HexagonF(0.f, -1.f,  1.f) // S  -> -R
   , HexagonF(-1.f,  0.f,  1.f) // SW -> -Q
   , HexagonF(-1.f,  1.f,  0.f) // NW -> -S
};

HexagonF::NeighborAccessor::NeighborAccessor(const HexagonF * const Parent)
   : parent(Parent)
{

}

HexagonF HexagonF::NeighborAccessor::operator[] (const int index) const
{
   checkf(index >= 0 && index < HexagonF::CoordCount, TEXT("HexagonF::NeighborAccessor[] index out of range (%d)"), index);
   return *parent + NeighborVectors[index];
}

HexagonF::HexagonF()
   : q(0.f), r(0.f), s(0.f), w(0.f), neighbors(this)
{

}

HexagonF::HexagonF(float Q, float R, float S, float W)
   : q(Q), r(R), s(S), w(W), neighbors(this)
{

}

HexagonF::HexagonF(const HexagonF& other)
   : q(other.q), r(other.r), s(other.s), w(other.w), neighbors(this)
{

}

HexagonF::HexagonF(const Hexagon& other)
   : q((float)other.q), r((float)other.r), s((float)other.s), w((float)other.w), neighbors(this)
{

}

bool HexagonF::operator==(const HexagonF& rhs) const
{
   return q == rhs.q && r == rhs.r && s == rhs.s && w == rhs.w;
}

bool HexagonF::operator!=(const HexagonF& rhs) const
{
   return q != rhs.q || r != rhs.r || s != rhs.s || w != rhs.w;
}

HexagonF& HexagonF::operator=(const HexagonF& rhs)
{
   q = rhs.q;
   r = rhs.r;
   s = rhs.s;
   w = rhs.w;
   return *this;
}

float& HexagonF::operator[](const int index)
{
   checkf(index >= 0 && index < HexagonF::CoordCount, TEXT("HexagonF::operator[] (non-const) was given an out of range index (%d)."), index);
   return v[index];
}

const float& HexagonF::operator[](const int index) const
{
   checkf(index >= 0 && index < HexagonF::CoordCount, TEXT("HexagonF::operator[] (const) was given an out of range index (%d)."), index);
   return v[index];
}

HexagonF HexagonF::operator-() const
{
   return HexagonF(-q, -r, -s, w); //w is not negated, it is either 0 or 1
}

HexagonF HexagonF::operator+(const HexagonF& rhs) const
{
   //w0 means it's a direction (vector), w1 means its a point (vertex)
   // two points can not be added together logically (LA + Chicago = ?)
   float W = r + rhs.w;
   checkf(W == 0 || W == 1, TEXT("HexagonF::operator+ Two 'points' (w=1) can not be added together."));
   return HexagonF(q + rhs.q, r + rhs.r, s + rhs.s, W);
}

HexagonF HexagonF::operator-(const HexagonF& rhs) const
{
   //w0 means it's a direction (vector), w1 means its a point (vertex)
   // a point can not be subtracted from a vector ( 5 miles north minus Chicago = ?)
   float W = r - rhs.w;
   checkf(W == 0.f || W == 1.f, TEXT("HexagonF::operator- A 'point' (w=1) can not be subtracted from a vector (w=0). Did you mean to do this operation in reverse (point - vector)?"));
   return HexagonF(q - rhs.q, r - rhs.r, s - rhs.s, W);
}

HexagonF HexagonF::operator*(const float scalar) const
{
   checkf(w == 0, TEXT("HexagonF::operator* Scaling a point (w=1) doesn't make sense. Did you mean to use a vector (w=0)?"));
   return HexagonF(q * scalar, r * scalar, s * scalar, w);
}

//Friend function
HexagonF operator*(const float& scalar, const HexagonF& hex)
{
   return hex * scalar;
}

HexagonF& HexagonF::operator+=(const HexagonF& rhs)
{
   //w=0 means it's a direction (vector), w=1 means its a point (vertex)
   // two points can not be added together logically (LA + Chicago = ?)
   float W = r + rhs.w;
   checkf(W == 0.f || W == 1.f, TEXT("HexagonF::operator+= Two 'points' (w=1) can not be added together."));
   q += rhs.q;
   r += rhs.r;
   s += rhs.s;
   w = W;
   return *this;
}

HexagonF& HexagonF::operator-=(const HexagonF& rhs)
{
   //w0 means it's a direction (vector), w1 means its a point (vertex)
   // a point can not be subtracted from a vector ( 5 miles north minus Chicago = ?)
   float W = r - rhs.w;
   checkf(W == 0.f || W == 1.f, TEXT("HexagonF::operator-= A 'point' (w=1) can not be subtracted from a vector (w=0). Did you mean to do this operation in reverse (point - vector)?"));
   q -= rhs.q;
   r -= rhs.r;
   s -= rhs.s;
   w = W;
   return *this;
}

HexagonF& HexagonF::operator*=(const float scalar)
{
   checkf(w == 0, TEXT("HexagonF::operator*= Scaling a point (w=1) doesn't make sense. Did you mean to use a vector (w=0)?"));
   q *= scalar;
   r *= scalar;
   s *= scalar;
   return *this;
}

float HexagonF::Magnatude() const
{
   checkf(w == 0, TEXT("HexagonF::Magnatude doesn't make sense on a point (w=1). Did you mean to use a vector (w=0)?"));
   float mag = (FMath::Abs(q) + FMath::Abs(r) + FMath::Abs(s)) / 2.f;
   return mag;
}

int Hexagon::DistanceTo(const Hexagon& rhs) const
{
   checkf(w == 1.f && rhs.w == 1.f, TEXT("Hexagon::DistanceTo doesn't make sense with vectors (w=0). Did you mean to use points (w=1)?"));
   int distance = (FMath::Abs(rhs.q - q) + FMath::Abs(rhs.r - r) + FMath::Abs(rhs.s - s)) / 2.f;
   return distance;
}


bool HexagonF::IsPoint() const
{
   return w == 1.f;
}

bool HexagonF::IsVector() const
{
   return w == 0.f;
}

