#include "plane.h"
#include <iostream>

namespace Cme
{
    ///////////////////////////////////////////////////////////////////////////////
    // Plane.cpp
    // =========
    // class for a 3D plane with normal vector (a,b,c) and a point (x0,y0,z0)
    // ax + by + cz + d = 0,  where d = -(ax0 + by0 + cz0)
    //
    // NOTE:
    // 1. The default plane is z = 0 (a plane on XY axis)
    // 2. The distance is the length from the origin to the plane
    //
    // Dependencies: Vector3, Line
    //
    //  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
    // CREATED: 2016-01-19
    // UPDATED: 2017-06-29
    ///////////////////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////////////////
    // ctors
    // default plane is z = 0 (a plane on XY-axis)
    ///////////////////////////////////////////////////////////////////////////////
    Plane::Plane() : normal(glm::vec3(0, 0, 1)), d(0), normalLength(1), distance(0)
    {
    }

    Plane::Plane(float a, float b, float c, float d)
    {
        set(a, b, c, d);
    }

    Plane::Plane(const glm::vec3& normal, const glm::vec3& point)
    {
        set(normal, point);
    }



    ///////////////////////////////////////////////////////////////////////////////
    // print itself with 4 coefficients of normalized plane equation
    // aX + bY + cZ + d = 0
    // where (a, b, c) is unit normal vector, d = -(ax0 + by0 + cz0)
    ///////////////////////////////////////////////////////////////////////////////
    void Plane::printSelf() const
    {
        std::cout << "Plane(" << normal.x << ", " << normal.y << ", " << normal.z
            << ", " << d << ")" << std::endl;
    }



    ///////////////////////////////////////////////////////////////////////////////
    // setters
    ///////////////////////////////////////////////////////////////////////////////
    void Plane::set(float a, float b, float c, float d)
    {
        normal.x = a;
        normal.y = b;
        normal.z = c;

        this->d = d;

        // compute distance
        normalLength = sqrtf(a * a + b * b + c * c);
        distance = -d / normalLength;
    }

    void Plane::set(const glm::vec3& normal, const glm::vec3& point)
    {
        this->normal = normal;
        normalLength = normal.length();        
        d = -glm::dot(normal, point);  // -(a*x0 + b*y0 + c*z0)
        distance = -d / normalLength;
    }



    ///////////////////////////////////////////////////////////////////////////////
    // compute the shortest distance from a given point P to the plane
    // Note: The distance is signed. If the distance is negative, the point is in
    // opposite side of the plane.
    //
    // D = (a * Px + b * Py + c * Pz + d) / sqrt(a*a + b*b + c*c)
    // reference: www.songho.ca/math/plane.html
    ///////////////////////////////////////////////////////////////////////////////
    float Plane::getDistance(const glm::vec3& point)
    {
        float dot = glm::dot(normal, point);
        return (dot + d) / normalLength;
    }



    ///////////////////////////////////////////////////////////////////////////////
    // normalize
    // divide each coefficient by the length of normal
    ///////////////////////////////////////////////////////////////////////////////
    void Plane::normalize()
    {
        float lengthInv = 1.0f / normalLength;
        normal *= lengthInv;
        normalLength = 1.0f;
        d *= lengthInv;
        distance = -d;
    }



    ///////////////////////////////////////////////////////////////////////////////
    // find the intersect point
    // substitute a point on the line to the plane equation, then solve for alpha
    // a point on a line: (x0 + x*t, y0 + y*t, z0 + z*t)
    // plane: a*X + b*Y + c*Z + d = 0
    //
    // a*(x0 + x*t) + b*(y0 + y*t) + c*(z0 + z*t) + d = 0
    // a*x0 + a*x*t + b*y0 + b*y*t + c*z0 + c*z*t + d = 0
    // (a*x + b*x + c*x)*t = -(a*x0 + b*y0 + c*z0 + d)
    //
    // t = -(a*x0 + b*y0 + c*z0 + d) / (a*x + b*x + c*x)
    ///////////////////////////////////////////////////////////////////////////////
    glm::vec3 Plane::intersect(const Line& line) const
    {
        // from line = p + t * v
        glm::vec3 p = line.getPoint();        // (x0, y0, z0)
        glm::vec3 v = line.getDirection();    // (x,  y,  z)

        // dot products  
        float dot1 = glm::dot(normal, p);   // a*x0 + b*y0 + c*z0       
        float dot2 = glm::dot(normal, v);   // a*x + b*y + c*z

        // if denominator=0, no intersect
        if (dot2 == 0)
            return glm::vec3(NAN, NAN, NAN);

        // find t = -(a*x0 + b*y0 + c*z0 + d) / (a*x + b*y + c*z)
        float t = -(dot1 + d) / dot2;

        // find intersection point
        return p + (v * t);
    }



    ///////////////////////////////////////////////////////////////////////////////
    // find the intersection line of 2 planes
    // P1: N1 dot p + d1 = 0 (a1*X + b1*Y + c1*Z + d1 = 0)
    // P2: N2 dot p + d2 = 0 (a2*X + b2*Y + c2*Z + d2 = 0)
    //
    // L: p0 + a*V where
    // V is the direction vector of intersection line = (a1,b1,c1) x (a2,b2,c2)
    // p0 is a point, which is on the L and both P1 and P2 as well
    //
    // p0 can be found by solving a linear system of 3 planes
    // P1: N1 dot p + d1 = 0     (given)
    // P2: N2 dot p + d2 = 0     (given)
    // P3: V dot p = 0           (chosen where d3=0)
    //
    // Use the formula for intersecting 3 planes to find p0;
    // p0 = ((-d1*N2 + d2*N1) x V) / V dot V
    ///////////////////////////////////////////////////////////////////////////////
    Line Plane::intersect(const Plane& rhs) const
    {
        // find direction vector of the intersection line
        glm::vec3 v = glm::cross(normal, rhs.getNormal());

        // if |direction| = 0, 2 planes are parallel (no intersect)
        // return a line with NaN
        if (v.x == 0 && v.y == 0 && v.z == 0)
            return Line(glm::vec3(NAN, NAN, NAN), glm::vec3(NAN, NAN, NAN));

        // find a point on the line, which is also on both planes
        // choose simple plane where d=0: ax + by + cz = 0                 
        float dot = glm::dot(v, v);                   // V dot V
        glm::vec3 n1 = rhs.getD() * normal;           // d2 * N1
        glm::vec3 n2 = -d * rhs.getNormal();          //-d1 * N2    
        glm::vec3 p = glm::cross(n1 + n2, v) / dot;   // (d2*N1-d1*N2) X V / V dot V

        return Line(v, p);
    }



    ///////////////////////////////////////////////////////////////////////////////
    // determine if it intersects with the line
    ///////////////////////////////////////////////////////////////////////////////
    bool Plane::isIntersected(const Line& line) const
    {
        // direction vector of line
        glm::vec3 v = line.getDirection();

        // dot product with normal of the plane
        float dot = glm::dot(normal, v);  // a*Vx + b*Vy + c*Vz

        if (dot == 0)
            return false;
        else
            return true;
    }



    ///////////////////////////////////////////////////////////////////////////////
    // determine if it intersects with the other plane
    ///////////////////////////////////////////////////////////////////////////////
    bool Plane::isIntersected(const Plane& plane) const
    {
        // check if 2 plane normals are same direction
        glm::vec3 cross = glm::cross(normal, plane.getNormal());
        
        if (cross.x == 0 && cross.y == 0 && cross.z == 0)
            return false;
        else
            return true;
    }

}
