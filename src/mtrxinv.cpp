#include <cmath>

typedef float MATRIX3[9];
    typedef float MATRIX4[16];
    typedef float VFLOAT;

void	m4_submat(const MATRIX4 mr, MATRIX3 mb, int i, int j ) {
      int di, dj, si, sj;
      // loop through 3x3 submatrix
      for( di = 0; di < 3; di ++ ) {
        for( dj = 0; dj < 3; dj ++ ) {
          // map 3x3 element (destination) to 4x4 element (source)
          si = di + ( ( di >= i ) ? 1 : 0 );
          sj = dj + ( ( dj >= j ) ? 1 : 0 );
          // copy element
          mb[di * 3 + dj] = mr[si * 4 + sj];
        }
      }
    }

  //The determinant of a 4x4 matrix can be calculated as follows:

  VFLOAT m3_det(const MATRIX3 mat )
      {
      VFLOAT det;
      det = mat[0] * ( mat[4]*mat[8] - mat[7]*mat[5] )
          - mat[1] * ( mat[3]*mat[8] - mat[6]*mat[5] )
          + mat[2] * ( mat[3]*mat[7] - mat[6]*mat[4] );
      return( det );
      }

    VFLOAT m4_det(const MATRIX4 mr )
      {
      VFLOAT  det, result = 0, i = 1;
      MATRIX3 msub3;
      int     n;
      for ( n = 0; n < 4; n++, i *= -1 )
        {

        m4_submat( mr, msub3, 0, n );
        det     = m3_det( msub3 );
        result += mr[n] * det * i;
        }
      return( result );
      }

  //And the inverse can be calculated as follows:

  void m4_identity(MATRIX4 mr )
  {
      mr[0] = 1;
    mr[1] = 0;
    mr[2] = 0;
    mr[3] = 0;
    mr[4] = 0;
    mr[5] = 1;
    mr[6] = 0;
    mr[7] = 0;
    mr[8] = 0;
    mr[9] = 0;
    mr[10] = 1;
    mr[11] = 0;
    mr[12] = 0;
    mr[13] = 0;
    mr[14] = 0;
    mr[15] = 1;
}

    int m4_inverse( MATRIX4 mr, const MATRIX4 ma )
      {

      VFLOAT  mdet = m4_det( ma );
      MATRIX3 mtemp;
      int     i, j, sign;
      if ( fabs( mdet ) < 0.0005 )
      {
        m4_identity( mr );
        return( 0 );
      }
      for ( i = 0; i < 4; i++ )
        for ( j = 0; j < 4; j++ )
          {
          sign = 1 - ( (i +j) % 2 ) * 2;
          m4_submat( ma, mtemp, i, j );
          mr[i+j*4] = ( m3_det( mtemp ) * sign ) / mdet;
          }
      return( 1 );
      }

void mtrxinv(float* mtrxret, const float* mtrx)
{
    m4_inverse(mtrxret, mtrx);
}
