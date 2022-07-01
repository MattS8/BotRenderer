#pragma once
#include <math.h>
#define M_PI   3.14159265358979323846264338327950288
#define uint unsigned int

#define degToRad(angleInDegrees) ((angleInDegrees) * M_PI / 180.0)
#define radToDeg(angleInRadians) ((angleInRadians) * 180.0 / M_PI)

namespace matrixMath
{
	struct Matrix3x3 {
		float matrix[3][3];

		float(&operator[](int i))[3]
		{
			return matrix[i];
		}

			Matrix3x3 operator*(int scalar)
		{
			Matrix3x3 newMatrix;

			for (int i = 0; i < 3; ++i)
			{
				for (int j = 0; j < 3; ++j)
				{
					newMatrix[i][j] = matrix[i][j] * scalar;
				}
			}

			return newMatrix;
		}

		Matrix3x3 operator*(Matrix3x3 matrix)
		{
			Matrix3x3 newMatrix;

			for (int i = 0; i < 3; ++i)
			{
				for (int j = 0; j < 3; ++j)
				{
					float sum = 0;
					for (int k = 0; k < 3; ++k)
						sum += this->matrix[i][k] * matrix[k][j];
					newMatrix[i][j] = sum;
				}
			}

			return newMatrix;
		}
	};

	struct Matrix1x3 {
		float matrix[3];

		Matrix1x3 operator*(int scalar)
		{
			Matrix1x3 newMatrix;
			for (int i = 0; i < 3; ++i)
				newMatrix.matrix[i] = matrix[i] * scalar;

			return newMatrix;
		}

		Matrix1x3 operator*(Matrix3x3 matrix3x3)
		{
			Matrix1x3 newMatrix;
			for (int j = 0; j < 3; ++j)
			{
				float sum = 0;
				for (int k = 0; k < 3; ++k)
					sum += matrix[k] * matrix3x3.matrix[k][j];
				newMatrix.matrix[j] = sum;
			}

			return newMatrix;
		}
	};

	struct Matrix4x4 {
		float matrix[4][4];

		Matrix4x4 RotateY(float degrees)
		{
			float radians = degToRad(degrees);
			Matrix4x4 rotationMatrix =
			{
				cos(radians), 0, sin(radians), 0,
				0, 1, 0, 0,
				-sin(radians), 0, cos(radians), 0,
				0, 0, 0, 1
			};

			*this = rotationMatrix * *this;

			return *this;
		}

		Matrix4x4 RotateX(float degrees)
		{
			float radians = degToRad(degrees);
			Matrix4x4 rotationMatrix =
			{
				1, 0, 0, 0,
				0, cos(radians), -sin(radians), 0,
				0, sin(radians), cos(radians), 0,
				0, 0, 0, 1
			};

			*this = *this * rotationMatrix;

			return *this;
		}

		Matrix4x4 RotateZ(float degrees)
		{
			float radians = degToRad(degrees);
			Matrix4x4 rotationMatrix =
			{
				cos(radians), -sin(radians), 0,
				sin(radians), cos(radians), 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1
			};

			*this = rotationMatrix * *this;

			return *this;
		}

		Matrix4x4 Translate(float x, float y, float z)
		{
			Matrix4x4 translationMatrix =
			{
				1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				x, y, z, 1
			};

			*this = translationMatrix * *this;

			return *this;
		}

		Matrix4x4 operator*(Matrix4x4 matrix)
		{
			Matrix4x4 newMatrix;

			for (int i = 0; i < 4; ++i)
			{
				for (int j = 0; j < 4; ++j)
				{
					float sum = 0;
					for (int k = 0; k < 4; ++k)
						sum += this->matrix[i][k] * matrix[k][j];
					newMatrix[i][j] = sum;
				}
			}

			return newMatrix;
		}

		float(&operator[](int i))[4]
		{
			return matrix[i];
		}
	};

	const Matrix4x4 IdentityMatrix4x4 = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	class Vertex {
	public:
		float xyzw[4];

		inline float x() { return xyzw[0]; }
		inline float y() { return xyzw[1]; }
		inline float z() { return xyzw[2]; }
		inline float w() { return xyzw[3]; }

		Vertex(float x = 0, float y = 0, float z = 0, float w = 1)
		{
			xyzw[0] = x;
			xyzw[1] = y;
			xyzw[2] = z;
			xyzw[3] = w;
		}

		float& operator[](int i)
		{
			return xyzw[i];
		}

		Vertex operator*(Matrix3x3 matrix)
		{
			Vertex vertex = *this;
			for (int j = 0; j < 3; ++j)
			{
				float sum = 0;
				for (int k = 0; k < 3; ++k)
					sum += xyzw[k] * matrix.matrix[k][j];
				vertex.xyzw[j] = sum;
			}

			return vertex;
		}

		Vertex operator*(Matrix4x4 matrix)
		{
			Vertex vertex = *this;
			for (int j = 0; j < 4; ++j)
			{
				float sum = 0;
				for (int k = 0; k < 4; ++k)
				{
					if (matrix.matrix[k][j] == 56)
					{
						int i = 0;
					}
					sum += xyzw[k] * matrix.matrix[k][j];
				}
				vertex.xyzw[j] = sum;
			}

			return vertex;
		}


	};
}