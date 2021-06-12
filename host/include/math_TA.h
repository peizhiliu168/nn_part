#ifndef TA_MY_TEST_H_MATH
#define TA_MY_TEST_H_MATH

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X, Y) (((X) > (Y)) ? (Y) : (X))

/* math */
#define PI 3.14159265358979323846
#define e  2.7182818284590452354
#define ln_2 0.69314718055994530942
#define ln_10 2.30258509299404568402
#define first_aim_money 1000000000.0f
#define inf 1e10000f

#define fabs(a) ((a)>0?(a):(-(a)))

float ta_max(float a, float b);
double ta_pow(double a,int n);
double ta_eee(double x);
double ta_exp(double x);
int ta_floor(double x);
double ta_sqrt(double x);
double ta_ln(double x);
double ta_log(double a,double N);
double ta_sin(double x);
double ta_cos(double x);
double ta_tan(double x);

void reverse(char *str, int len);
int intToStr(int x, char str[], int d);
void ftoa(float n, char *res, int afterpoint);
void bubble_sort_top(float *arr, int len);

double libm_log (double x);

static double
ln2_hi  =  6.93147180369123816490e-01,	/* 3fe62e42 fee00000 */
ln2_lo  =  1.90821492927058770002e-10,	/* 3dea39ef 35793c76 */
two54   =  1.80143985094819840000e+16,  /* 43500000 00000000 */
Lg1 = 6.666666666666735130e-01,  /* 3FE55555 55555593 */
Lg2 = 3.999999999940941908e-01,  /* 3FD99999 9997FA04 */
Lg3 = 2.857142874366239149e-01,  /* 3FD24924 94229359 */
Lg4 = 2.222219843214978396e-01,  /* 3FCC71C5 1D8E78AF */
Lg5 = 1.818357216161805012e-01,  /* 3FC74664 96CB03DE */
Lg6 = 1.531383769920937332e-01,  /* 3FC39A09 D078C69F */
Lg7 = 1.479819860511658591e-01;  /* 3FC2F112 DF3E5244 */

static double zero   =  0.0;

// little endian
#define __HI(x) *(1+(int*)&x)
#define __LO(x) *(int*)&x
#define __HIp(x) *(1+(int*)x)
#define __LOp(x) *(int*)x


#endif /*TA_MY_TEST_H_MATH*/
