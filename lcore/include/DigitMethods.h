// ��������� ��������� ������

#pragma once

#include <math.h>
#include <points.h>

// ���������� ������ ����������������� ������������ � ��������� �����
// �.�. ����� ������, ����� ��������� ��������� �� ������� �� ���� ����� ����������
// from_zero - ������ �������� ����� ����
DLL_OBJ Line<double> CalcMiddleSqrLineFunc( int count, Point2D<double>* points, bool from_zero = false );
