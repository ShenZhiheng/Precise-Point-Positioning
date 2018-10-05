#include "CMatrix.h"
#include "Structs.h"
#include <iostream>

using namespace std;

int CMatrix::count=0;
CMatrix::CMatrix(int mRows,int mCols)
{
	Rows=mRows;
	Cols=mCols;
	element=new double* [Rows];
	for (int i=0;i<Rows;i++)
	{
		element[i] = new double [Cols];
	}
	for (int i=0;i<Rows;i++)
	{
		for (int j=0;j<Cols;j++)
		{
			element[i][j]=0;
		}
	}
	
}
CMatrix::CMatrix(double* pArray,int mRows,int mCols)
{
	Rows=mRows;
	Cols=mCols;
	element=new double* [Rows];
	for (int i=0;i<Rows;i++)
	{
		element[i] = new double [Cols];
	}
	for (int i=0;i<Rows;i++)
	{
		for (int j=0;j<Cols;j++)
		{
			element[i][j]=*(pArray+i*Cols+j);
		}
	}
}

CMatrix::CMatrix(std::vector<double> vec)
{
	Rows = vec.size();
	Cols = 1;
	element = new double*[Rows];
	for (int i = 0; i<Rows; i++)
	{
		element[i] = new double[Cols];
	}
	for (int i = 0; i < Rows; i++)
	{
		element[i][0] = vec[i];
	}
}

CMatrix::CMatrix(const CMatrix& CMat)
{
	Rows=CMat.Rows;
	Cols=CMat.Cols;
	element=new double* [Rows];
	for (int i=0;i<Rows;i++)
	{
		element[i] = new double [Cols];
	}
	for (int i=0;i<Rows;i++)
	{
		for (int j=0;j<Cols;j++)
		{
			element[i][j]=CMat.element[i][j];
		}
	}
}
CMatrix::~CMatrix()
{
	count++;
	for (int i=0;i<Rows;i++)
	{
		delete[]element[i];
	}
	delete []element;
	//cout<<"dispose this point!!!"<<endl;
}
double CMatrix::GetNum(int x,int y)const
{
	if (x<Rows&&y<Cols)
	{
		return element[x][y];
	}
	else
	{
		cout<<"out of range!!!"<<endl;
		return 0;
	}

}
void CMatrix::SetNum(int x,int y,double t)
{
	if (x<Rows&&y<Cols)
	{
		element[x][y]=t;
	}
	else
	{
		cout<<"out of range!!!"<<endl;
	}
}

void CMatrix::SetDiag(initializer_list<double> il)
{
	int i = 0;
	for (auto beg = il.begin(); i < il.size(), beg != il.end(); beg++, i++)
	{
		this->SetNum(i, i, *beg);
	}
}

void CMatrix::SetDiag2(initializer_list<double> il)
{
	int i = 0;
	for (auto beg = il.begin(); i < il.size(), beg != il.end(); beg++, i++)
	{
		this->SetNum(i, i, SQR(*beg));
	}
}

CMatrix& CMatrix::operator=(const CMatrix& CMat)
{
	if (this==&CMat)
	{
		return *this;
	}
	//先销毁
	for (int i=0;i<Rows;i++)
	{
		delete[]element[i];
	}
	delete []element;
	//再new一个与参数矩阵行列数相同的矩阵
	Rows=CMat.Rows;
	Cols=CMat.Cols;
	element=new double* [Rows];
	for (int i=0;i<Rows;i++)
	{
		element[i] = new double [Cols];
	}
	//赋值
	for (int i=0;i<Rows;i++)
	{
		for (int j=0;j<Cols;j++)
		{
			element[i][j]=CMat.element[i][j];
		}
	}
	return *this;
}
CMatrix& CMatrix::operator+=(const CMatrix& CMat)
{
	if (Rows==CMat.Rows&&Cols==CMat.Cols)
	{
		for (int i=0;i<Rows;i++)
		{
			for (int j=0;j<Cols;j++)
			{
				element[i][j]+=CMat.element[i][j];
			}
		}
		return *this;
	}
	else 
	{
		cout<<"This Matrix is not matched!"<<endl;
		return *this;
	}
}
CMatrix CMatrix::operator+(const CMatrix& CMat)const
{
	if (Rows==CMat.Rows&&Cols==CMat.Cols)
	{
		CMatrix cmat_out(Rows,Cols);
		for (int i=0;i<Rows;i++)
		{
			for (int j=0;j<Cols;j++)
			{
				cmat_out.element[i][j]=element[i][j]+CMat.element[i][j];
				//element[i][j]=22;
			}
		}
		return cmat_out;
	}
	else 
	{
		cout<<"This Matrix is not matched!"<<endl;
		return *this;
	}
}
CMatrix CMatrix::operator-(const CMatrix& CMat)const
{
	if (Rows==CMat.Rows&&Cols==CMat.Cols)
	{
		CMatrix cmat_out(Rows,Cols);
		for (int i=0;i<Rows;i++)
		{
			for (int j=0;j<Cols;j++)
			{
				cmat_out.element[i][j]=element[i][j]-CMat.element[i][j];
			}
		}
		return cmat_out;
	}
	else 
	{
		cout<<"This Matrix is not matched!"<<endl;
		return *this;
	}
}
CMatrix CMatrix::operator-()const
{
	CMatrix cmat_out(Rows,Cols);
	for (int i=0;i<Rows;i++)
	{
		for (int j=0;j<Cols;j++)
		{
			cmat_out.element[i][j]=-element[i][j];
		}
	}
	return cmat_out;
}
CMatrix CMatrix::operator*(const CMatrix& CMat)const
{
	if (Cols==CMat.Rows)
	{
		CMatrix cmat_out(Rows,CMat.Cols);
		for (int i=0;i<Rows;i++)
		{
			for (int j=0;j<CMat.Cols;j++)
			{
				for (int k=0;k<Cols;k++)
				{
					cmat_out.element[i][j]+=element[i][k]*CMat.element[k][j];
				}
				
			}
		}
		return cmat_out;
	}
	else
	{
		cout<<"This Matrix is not matched!"<<endl;
		return *this;
	}
	
}
CMatrix CMatrix::operator*(int n)const
{
	CMatrix cmat_out(Rows,Cols);
	for (int i=0;i<Rows;i++)
	{
		for (int j=0;j<Cols;j++)
		{
			cmat_out.element[i][j]=3*element[i][j];
		}
	}
	return cmat_out;
}
CMatrix CMatrix::operator^(int n)const
{
	if (Rows==Cols)
	{
		CMatrix cmat_out(Rows,Cols);
		for (int i=0;i<Rows;i++)
		{
			for (int j=0;j<Cols;j++)
			{
				if (i==j)
					cmat_out.element[i][j]=1;//初始为单位阵
				else
					cmat_out.element[i][j]=0;
			}
		}
		for (int i=0;i<n;i++)
		{
			cmat_out=cmat_out*(*this);
		}
		return cmat_out;
	}
	else
	{
		cout<<"The Matrix is not square!!!"<<endl;
		return *this;
	}
	
}
CMatrix CMatrix::Row_Exchange(int m,int n)const
{
	if (m<Rows&&n<Rows)
	{
		CMatrix cmat_out(*this);
		double* pRow_temp=new double[Cols];
		for (int i=0;i<Cols;i++)
		{
			pRow_temp[i]=cmat_out.element[m][i];
			cmat_out.element[m][i]=cmat_out.element[n][i];
			cmat_out.element[n][i]=pRow_temp[i];
		}
		delete[]pRow_temp;
		return cmat_out;
	}
	else
	{
		cout<<"The Row is out of range !"<<endl;
		return *this;
	}

}
CMatrix CMatrix::Col_Exchange(int m,int n)const
{
	if (m<Cols&&n<Cols)
	{
		CMatrix cmat_out(*this);
		double* pRow_temp=new double[Rows];
		for (int i=0;i<Rows;i++)
		{
			pRow_temp[i]=cmat_out.element[i][m];
			cmat_out.element[i][m]=cmat_out.element[i][n];
			cmat_out.element[i][n]=pRow_temp[i];
		}
		delete[]pRow_temp;
		return cmat_out;
	}
	else
	{
		cout<<"The Col is out of range !"<<endl;
		return *this;
	}
}
CMatrix CMatrix::RowAdd(int m,int n,double t)const
{
	if (m<Rows&&n<Rows)
	{
		CMatrix cmat_out(*this);
		for (int i=0;i<Cols;i++)
		{
			cmat_out.element[n][i]+=cmat_out.element[m][i]*t;
		}
		return cmat_out;
	}
	else
	{
		cout<<"The Row is out of range !"<<endl;
		return *this;
	}
}
CMatrix CMatrix::RowMulti(int m,double t)const
{
	if (m<Rows)
	{
		CMatrix cmat_out(*this);
		for (int i=0;i<Cols;i++)
		{
			cmat_out.element[m][i]=t*element[m][i];
		}
		return cmat_out;
	}
	else
	{
		cout<<"out of range!"<<endl;
		return *this;
	}
}
CMatrix CMatrix::E_Matrix()const
{
	if (Rows==Cols)
	{
		CMatrix cmat_out(Rows,Cols);
		for (int i=0;i<Rows;i++)
		{
			for (int j=0;j<Cols;j++)
			{
				if (i==j) cmat_out.element[i][j]=1;
				else cmat_out.element[i][j]=0;
			}
		}
		return cmat_out;
	}
	else
		return *this;
}
CMatrix CMatrix::Abs()const
{
	CMatrix cmat_out(Rows,Cols);
	for (int i=0;i<Rows;i++)
	{
		for (int j=0;j<Cols;j++)
		{
			cmat_out.element[i][j]=abs(element[i][j]);
		}
	}
	return cmat_out;
}
CMatrix CMatrix::Transpose()const
{
	CMatrix cmat_out(Cols,Rows);
	for (int i=0;i<Cols;i++)
	{
		for (int j=0;j<Rows;j++)
		{
			cmat_out.element[i][j]=element[j][i];
		}
	}
	return cmat_out;
}
CMatrix CMatrix::Inverse()const
{
	if (Rows==Cols&&GetDet()!=0)
	{
		CMatrix cmat_out(*this);
		CMatrix E_cmat=E_Matrix();//初始化一个同样大小的单位阵
		for (int i=0;i<Rows;i++)
		{
			//找到第i列绝对值最大的数
			int Row_Max=i;
			double Max_Value=abs(cmat_out.element[i][i]);
			for (int j=i;j<Rows;j++)
			{
				if (abs(cmat_out.element[j][i])>Max_Value)
				{
					Row_Max=j;
					Max_Value=abs(cmat_out.element[j][i]);
				}
			}
			//将第i行和找到最大数那一行Row_Max交换,同时将单位阵做相同初等变换
			if (Row_Max!=i)
			{
				cmat_out=cmat_out.Row_Exchange(i,Row_Max);
				E_cmat=E_cmat.Row_Exchange(i,Row_Max);
			}
			//将第i行做初等行变换，将第一个非0元素化为1
			double t=1.0/cmat_out.element[i][i];
			cmat_out=cmat_out.RowMulti(i,t);
			E_cmat=E_cmat.RowMulti(i,t);
			//消元
			for (int j=0;j<Rows;j++)
			{
				//到本行后跳过
				if (j==i)continue;
				else
				{
					t=-cmat_out.element[j][i];
					cmat_out=cmat_out.RowAdd(i,j,t);
					E_cmat=E_cmat.RowAdd(i,j,t);
				}
			}
		}

		return E_cmat;
	}
	else
	{
		cout<<"The Matrix is not square!!!"<<endl;
		return *this;
	}
}
double CMatrix::GetDet()const
{
	if (Rows==Cols)
	{
		CMatrix cmat_out(*this);
		double result=1;
		for (int i=0;i<Rows;i++)
		{
			//找到第i列绝对值最大的数
			int Row_Max=i;
			double Max_Value=abs(cmat_out.element[i][i]);
			for (int j=i;j<Rows;j++)
			{
				if (abs(cmat_out.element[j][i])>Max_Value)
				{
					Row_Max=j;
					Max_Value=abs(cmat_out.element[j][i]);
				}
			}
			 //将第i行和找到最大数那一行Row_Max交换
			if (Row_Max!=i)
			{
				cmat_out=cmat_out.Row_Exchange(i,Row_Max);
				result*=-1;
			}
			//消元
			for (int j=i+1;j<Rows;j++)
			{
				double t=-cmat_out.element[j][i]/cmat_out.element[i][i];
				cmat_out=cmat_out.RowAdd(i,j,t);
			}
			
		}
		//对角线乘积
		for (int i=0;i<Rows;i++)
		{
			result*=cmat_out.element[i][i];
		}
		return result;
	}
	else
	{
		cout<<"Error!"<<endl;
		return 0;
	}
	

	
}
void CMatrix::Show()const
{
	for (int i=0;i<Rows;i++)
	{
		for (int j=0;j<Cols;j++)
		{
			cout<<element[i][j]<<' ';
		}
		cout<<endl;
	}
}

CMatrix CMatrix::E_Matrix(int m, int n)
{
	CMatrix E(m, n);
	for (int i = 0; i < m; i++)
		for (int j = 0; j < n; j++)
			if (i == j)
				E.SetNum(i, j, 1);
	return E;
}


