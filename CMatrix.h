#ifndef CMATRIX_H
#define CMATRIX_H
#include <vector>

class CMatrix
{
private:
	double** element;
	int Rows;//行
	int Cols;//列
public:
	static int count;

	CMatrix(int mRows=1,int mCols=1);
	CMatrix(double* pArray,int mRows,int mCols);
	CMatrix(std::vector<double> vec);
	CMatrix(const CMatrix& CMat);
	~CMatrix();

	int GetRows()const{ return Rows; }
	int GetCols()const{ return Cols; }
	double GetNum(int x, int y)const;
	void SetNum(int x, int y, double t);
	void SetDiag(std::initializer_list<double> il);
	void SetDiag2(std::initializer_list<double> il);
	CMatrix& operator=(const CMatrix& CMat);
	CMatrix& operator+=(const CMatrix& CMat);
	CMatrix operator+(const CMatrix& CMat)const;
	CMatrix operator-(const CMatrix& CMat)const;
	CMatrix operator-()const;                        // 取负
	CMatrix operator*(const CMatrix& CMat)const;     // 矩阵相乘
	CMatrix operator*(int n)const;                   // 数乘
	CMatrix operator^(int n)const;                   // 幂次方
	CMatrix Row_Exchange(int m,int n)const;          // 交换两行
	CMatrix Col_Exchange(int m,int n)const;          // 交换两列
	CMatrix RowAdd(int m,int n,double t)const;       // 将第m行乘t加到第n行
	CMatrix RowMulti(int m,double t)const;           // 第m行乘t
	CMatrix E_Matrix()const;                         // 创建一个同样大小的单位阵
	CMatrix Abs()const;
	CMatrix Transpose()const;
	CMatrix Inverse()const;
	double GetDet()const;
	void Show()const;

	static CMatrix E_Matrix(int m, int n);           // 创建一个单位阵
	
};

#endif