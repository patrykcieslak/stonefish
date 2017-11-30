#include "Ned.h"

Ned::Ned(const double lat, const double lon, const double height)
{
  // Save NED origin
  _init_lat = lat/180.0 * M_PI;
  _init_lon = lon/180.0 * M_PI;
  _init_h = height;

  // Compute ECEF of NED origin
  geodetic2Ecef(lat, lon, height, _init_ecef_x, _init_ecef_y, _init_ecef_z);

  // Compute ECEF to NED and NED to ECEF matrices
  double phiP = atan2(_init_ecef_z, sqrt(pow(_init_ecef_x, 2) + pow(_init_ecef_y, 2)));

  _ecef_to_ned_matrix = __nRe__(phiP, _init_lon);
  _ned_to_ecef_matrix = __nRe__(_init_lat, _init_lon).transpose();
}

void Ned::geodetic2Ecef(const double lat, const double lon, const double height,
                        double& x, double& y, double& z)
{
  // Convert geodetic coordinates to ECEF.
  // http://code.google.com/p/pysatel/source/browse/trunk/coord.py?r=22
  double lat_rad = lat/180.0 * M_PI;
  double lon_rad = lon/180.0 * M_PI;
  double xi = sqrt(1 - esq * sin(lat_rad) * sin(lat_rad));
  x = (a / xi + height) * cos(lat_rad) * cos(lon_rad);
  y = (a / xi + height) * cos(lat_rad) * sin(lon_rad);
  z = (a / xi * (1 - esq) + height) * sin(lat_rad);
}

void Ned::ecef2Geodetic(const double x, const double y, const double z,
                        double& lat, double& lon, double& height)
{
  // Convert ECEF coordinates to geodetic.
  // J. Zhu, "Conversion of Earth-centered Earth-fixed coordinates
  // to geodetic coordinates," IEEE Transactions on Aerospace and
  // Electronic Systems, vol. 30, pp. 957-961, 1994.

  double r = sqrt(x * x + y * y);
  double Esq = a * a - b * b;
  double F = 54 * b * b * z * z;
  double G = r * r + (1 - esq) * z * z - esq * Esq;
  double C = (esq * esq * F * r * r) / pow(G, 3);
  double S = __cbrt__(1 + C + sqrt(C * C + 2 * C));
  double P = F / (3 * pow((S + 1 / S + 1), 2) * G * G);
  double Q = sqrt(1 + 2 * esq * esq * P);
  double r_0 = -(P * esq * r) / (1 + Q) + sqrt(0.5 * a * a * (1 + 1.0 / Q) - P * (1 - esq) * z * z / (Q * (1 + Q)) - 0.5 * P * r * r);
  double U = sqrt(pow((r - esq * r_0), 2) + z * z);
  double V = sqrt(pow((r - esq * r_0), 2) + (1 - esq) * z * z);
  double Z_0 = b * b * z / (a * V);
  height = U * (1 - b * b / (a * V));
  lat = (atan((z + e1sq * Z_0) / r))/M_PI * 180.0;
  lon = (atan2(y, x))/M_PI * 180.0;
}

void Ned::ecef2Ned(const double x, const double y, const double z,
                   double& north, double& east, double& depth)
{
  // Converts ECEF coordinate pos into local-tangent-plane ENU
  // coordinates relative to another ECEF coordinate ref. Returns a tuple
  // (East, North, Up).

  Eigen::Vector3d vect, ret;
  vect(0) = x - _init_ecef_x;
  vect(1) = y - _init_ecef_y;
  vect(2) = z - _init_ecef_z;
  ret = _ecef_to_ned_matrix * vect;
  north = ret(0);
  east = ret(1);
  depth = -ret(2);
}

void Ned::ned2Ecef(const double north, const double east, const double depth,
                   double& x, double& y, double& z)
{
  // NED (north/east/down) to ECEF coordinate system conversion.
  Eigen::Vector3d ned, ret;
  ned(0) = north;
  ned(1) = east;
  ned(2) = -depth;
  ret = _ned_to_ecef_matrix * ned;
  x = ret(0) + _init_ecef_x;
  y = ret(1) + _init_ecef_y;
  z = ret(2) + _init_ecef_z;
}

void Ned::geodetic2Ned(const double lat, const double lon, const double height,
                       double& north, double& east, double& depth)
{
  // Geodetic position to a local NED system """
  double x, y, z;
  geodetic2Ecef(lat, lon, height, x, y, z);
  ecef2Ned(x, y, z, north, east, depth);
}

void Ned::ned2Geodetic(const double north, const double east, const double depth,
                       double& lat, double& lon, double& height)
{
  // Local NED position to geodetic
  double x, y, z;
  ned2Ecef(north, east, depth, x, y, z);
  ecef2Geodetic(x, y, z, lat, lon, height);
}

double Ned::__cbrt__(const double x)
{
  if (x >= 0.0)
  {
    return pow(x, 1.0/3.0);
  }
  else
  {
    return -pow(fabs(x), 1.0/3.0);
  }
}

Eigen::Matrix3d Ned::__nRe__(const double lat_rad, const double lon_rad)
{
  double sLat = sin(lat_rad);
  double sLon = sin(lon_rad);
  double cLat = cos(lat_rad);
  double cLon = cos(lon_rad);

  Eigen::Matrix3d ret;
  ret(0, 0) = -sLat*cLon;     ret(0, 1) = -sLat*sLon;     ret(0, 2) = cLat;
  ret(1, 0) = -sLon;          ret(1, 1) = cLon;           ret(1, 2) = 0.0;
  ret(2, 0) = cLat*cLon;      ret(2, 1) = cLat*sLon;      ret(2, 2) = sLat;

  return ret;
}