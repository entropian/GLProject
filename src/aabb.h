#ifndef AABB_H
#define AABB_H

#include <vector>
#include <cfloat>
#include "vec.h"

class AABB
{
public:
    AABB()
    {
    }

    AABB(const std::vector<Vec3> &positions)
    {
        calcAABB(positions);
    }
    
    void calcAABB(const std::vector<Vec3> &positions)
    {
        int xmin, xmax, ymin, ymax, zmin, zmax;
        extremePointsAlongDir(Vec3(1, 0, 0), positions, xmin, xmax);
        extremePointsAlongDir(Vec3(0, 1, 0), positions, ymin, ymax);
        extremePointsAlongDir(Vec3(0, 0, 1), positions, zmin, zmax);
        min = Vec3(positions[xmin][0], positions[ymin][1], positions[zmin][2]);
        max = Vec3(positions[xmax][0], positions[ymax][1], positions[zmax][2]);
    }

    void extremePointsAlongDir(Vec3 dir, const std::vector<Vec3>& positions, int &min, int &max)
    {
        float minproj = FLT_MAX;
        float maxproj = -FLT_MAX;

        for(int i = 0; i < positions.size(); i++)
        {
            float proj = dot(positions[i], dir);
            if(proj < minproj)
            {
                minproj = proj;
                min = i;
            }

            if(proj > maxproj)
            {
                maxproj = proj;
                max = i;
            }
        }
    }

    Vec3 getMin()
    {
        return min;
    }

    Vec3 getMax()
    {
        return max;
    }    
    
private:
    Vec3 min;
    Vec3 max;
};


#endif
