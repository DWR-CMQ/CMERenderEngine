#include "line.h"
#include <iostream>

namespace Cme
{
    ///////////////////////////////////////////////////////////////////////////////
    Line::Line(float slope, float intercept)
    {
        set(slope, intercept);
    }

    Line::Line(const glm::vec2& direction, const glm::vec2& point)
    {
        set(direction, point);
    }

    void Line::set(const glm::vec3& v, const glm::vec3& p)
    {
        this->direction = v;
        this->point = p;
    }

    void Line::set(const glm::vec2& v, const glm::vec2& p)
    {
        // convert 2D to 3D
        this->direction = glm::vec3(v.x, v.y, 0);
        this->point = glm::vec3(p.x, p.y, 0);
    }

    void Line::set(float slope, float intercept)
    {
        // convert slope-intercept form (2D) to parametric form (3D)
        this->direction = glm::vec3(1, slope, 0);
        this->point = glm::vec3(0, intercept, 0);
    }

    void Line::printSelf()
    {
        std::cout << "Line\n"
            << "====\n"
            << "Direction: " << direction.x << " " << direction.y << " " << direction.z << "\n"
            << "    Point: " << point.x << " " << point.y << " " << point.z << std::endl;
    }

    ///////////////////////////////////////////////////////////////////////////////
    // find the intersection point with the other line.
    // If no intersection, return a point with NaN in it.
    //
    // Line1 = p1 + aV1 (this)
    // Line2 = p2 + bV2 (other)
    //
    // Intersect:
    // p1 + aV1 = p2 + bV2
    //      aV1 = (p2-p1) + bV2
    //   aV1xV2 = (p2-p1)xV2
    //        a = (p2-p1)xV2 / (V1xV2)
    //        a = ((p2-p1)xV2).(V1xV2) / (V1xV2).(V1xV2)
    ///////////////////////////////////////////////////////////////////////////////
    glm::vec3 Line::intersect(const Line& line)
    {
        const glm::vec3 v2 = line.getDirection();
        const glm::vec3 p2 = line.getPoint();
        glm::vec3 result = glm::vec3(NAN, NAN, NAN);    // default with NaN

        // find v3 = (p2 - p1) x V2
        glm::vec3 v3 = glm::cross(p2 - point, v2);

        // find v4 = V1 x V2
        glm::vec3 v4 = glm::cross(direction, v2);

        // find (V1xV2) . (V1xV2)
        float dot = glm::dot(v4, v4);

        // if both V1 and V2 are same direction, return NaN point
        if (dot == 0)
            return result;

        // find a = ((p2-p1)xV2).(V1xV2) / (V1xV2).(V1xV2)
        float alpha = glm::dot(v3, v4) / dot;

        /*
        // if both V1 and V2 are same direction, return NaN point
        if(v4.x == 0 && v4.y == 0 && v4.z == 0)
            return result;

        float alpha = 0;
        if(v4.x != 0)
            alpha = v3.x / v4.x;
        else if(v4.y != 0)
            alpha = v3.y / v4.y;
        else if(v4.z != 0)
            alpha = v3.z / v4.z;
        else
            return result;
        */

        // find intersect point
        result = point + (alpha * direction);
        return result;
    }



    ///////////////////////////////////////////////////////////////////////////////
    // determine if it intersects with the other line
    ///////////////////////////////////////////////////////////////////////////////
    bool Line::isIntersected(const Line& line)
    {
        // if 2 lines are same direction, the magnitude of cross product is 0
        glm::vec3 v = glm::cross(direction, line.getDirection());
        if (v.x == 0 && v.y == 0 && v.z == 0)
            return false;
        else
            return true;
    }
}