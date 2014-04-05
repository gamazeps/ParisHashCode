// Source : http://www.linuxjournal.com/magazine/work-shell-calculating-distance-between-two-latitudelongitude-points


#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define EARTH_RADIUS       (6371.0072)
#define TORADS(degrees)    (degrees * (M_PI / 180))


double dist_meters(double lat1, double long1, double lat2, double long2){
   double dLat, dLong, a, c, d;

   lat1  = TORADS(lat1);
   long1 = TORADS(long1);
   lat2  = TORADS(lat2);
   long2 = TORADS(long2);

   dLat  = lat2 - lat1;
   dLong = long2 - long1;

   a = sin(dLat/2) * sin(dLat/2) +
       cos(lat1) * cos(lat2) * sin(dLong/2) * sin(dLong/2);
   c = 2 * atan2(sqrt(a), sqrt(1-a));

   // Dist is the distance in meters between two positions
   double dist = EARTH_RADIUS * c * 1000;
   return dist;

}
