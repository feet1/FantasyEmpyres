#pragma once
enum class HexDirectionIndex : int32
{
   //Explicitly indicate values since these are used as array indices
   //we assume all hexes are the flat-topped kind
   North = 0,
   NorthEast = 1,
   SouthEast = 2,
   South = 3,
   SouthWest = 4,
   NorthWest = 5
};

class HexagonFraction;
class Hexagon
{
public:
   class NeighborAccessor
   {
   public:
      Hexagon operator[] (const int index) const;
   private:
      NeighborAccessor(const Hexagon * const Parent);
      const Hexagon * const parent;
      friend class Hexagon;
   };

public:
   //Constructors
   Hexagon();
   Hexagon(int Q, int R, int S, int W=0);
   Hexagon(const Hexagon& other);
   explicit Hexagon(const HexagonFraction& other);

   //Equality
   bool operator==(const Hexagon& rhs) const;
   bool operator!=(const Hexagon& rhs) const;

   //Assignment
   Hexagon& operator=(const Hexagon& rhs);

   //array access to values
   int32& operator[] (const int index);
   const int32& operator[] (const int index) const;

   Hexagon operator-() const;

   Hexagon operator+(const Hexagon& rhs) const;
   Hexagon operator-(const Hexagon& rhs) const;
   Hexagon operator*(const float scalar) const;
   Hexagon operator*(const int scalar) const;
   Hexagon operator%(const  int mod) const;
   friend Hexagon operator*(const float& scalar, const Hexagon& hex);
   friend Hexagon operator*(const int& scalar, const Hexagon& hex);

   Hexagon& operator+=(const Hexagon& rhs);
   Hexagon& operator-=(const Hexagon& rhs);
   Hexagon& operator*=(const float scalar);
   Hexagon& operator*=(const int scalar);
   Hexagon& operator%=(const  int mod);

   int Magnatude() const;
   int DistanceTo(const Hexagon& rhs) const;
   bool IsPoint() const;
   bool IsVector() const;

public:
   static const int CoordCount = 4; // Q R S W

private:
   static const Hexagon NeighborVectors[SidesOnAHex];

private:
   union
   {
      struct { int32 q, r, s, w; };
      int32 v[CoordCount];
   };

public:
   const NeighborAccessor neighbors;
   friend class HexagonF;
};

class HexagonF
{
public:
   class NeighborAccessor
   {
   public:
      HexagonF operator[] (const int index) const;
   private:
      NeighborAccessor(const HexagonF * const Parent);
      const HexagonF * const parent;
      friend class HexagonF;
   };

public:
   //Constructors
   HexagonF();
   HexagonF(float Q, float R, float S, float W = 0.f);
   HexagonF(const HexagonF& other);
   explicit HexagonF(const Hexagon& other);

   //Equality
   bool operator==(const HexagonF& rhs) const;
   bool operator!=(const HexagonF& rhs) const;

   //Assignment
   HexagonF& operator=(const HexagonF& rhs);

   //array access to values
   float& operator[] (const int index);
   const float& operator[] (const int index) const;

   HexagonF operator-() const;

   HexagonF operator+(const HexagonF& rhs) const;
   HexagonF operator-(const HexagonF& rhs) const;
   HexagonF operator*(const float scalar) const;
   friend HexagonF operator*(const float& scalar, const HexagonF& hex);

   HexagonF& operator+=(const HexagonF& rhs);
   HexagonF& operator-=(const HexagonF& rhs);
   HexagonF& operator*=(const float scalar);

   float Magnatude() const;
   //The number of "tiles" away the other hex is
   int32 DistanceTo(const HexagonF& rhs) const;
   //The distance the fractional position is from this fractional position
   float DistanceToPrecice(const HexagonF& rhs) const;
   bool IsPoint() const;
   bool IsVector() const;

   HexagonF& Round();
   HexagonF AsRounded() const;

public:
   static const int CoordCount = 4; // Q R S W

private:
   static const HexagonF NeighborVectors[SidesOnAHex];

private:
   union
   {
      struct { float q, r, s, w; };
      float v[CoordCount];
   };

public:
   const NeighborAccessor neighbors;
   friend class Hexagon;
};
