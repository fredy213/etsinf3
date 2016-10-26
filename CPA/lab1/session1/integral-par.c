#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>

/* Funcion f(x) de la cual se quiere calcular la integral */
double f(double x)
{
//   return pow(10*x-14.0/3.0,3)-150*x+120;
   return M_PI/2.0*sin(x*M_PI);
}

/* Calculo de la integral de la funcion f. Variante 1 */
double calcula_integral1(double a, double b, int n)
{
   double h, s=0, result;
   int i;

   h=(b-a)/n;

   double t1, t2;
   t1 = omp_get_wtime();

   #pragma omp parallel for reduction(+:s)
   for (i=0; i<n; i++) {
      s+=f(a+h*(i+0.5));
   }

   t2 = omp_get_wtime();

   printf("time: %f\n", t2 - t1);

   result = h*s;
   return result;
}

/* Calculo de la integral de la funcion f. Variante 2 */
double calcula_integral2(double a, double b, int n)
{
   double x, h, s=0, result;
   int i;
   struct timespec t;

   t.tv_sec=0;
   t.tv_nsec=1000;

   h=(b-a)/n;

   #pragma omp parallel for reduction(+:s) private(x)
   for (i=0; i<n; i++) {
      x=a+h*(i+0.5);
      /* Duermo un poco. Si la paralelización está bien hecha
         esta cabezadita no afectara al resultado */
      nanosleep(&t,NULL);
      s+=f(x);
   }

   result = h*s;
   return result;
}

int main(int argc, char *argv[])
{
   double a, b, result;
   int n, variante;

   if (argc<2) {
      fprintf(stderr, "Numero de argumentos incorrecto\n");
      return 1;
   }
   if (argc>2) n=atoi(argv[2]);
   else n=1000;
   a=0;
   b=1;

   variante=atoi(argv[1]);
   switch (variante) {
      case 1:
         result = calcula_integral1(a,b,n);
         break;
      case 2:
         result = calcula_integral2(a,b,n);
         break;
      default:
         fprintf(stderr, "Numero de variante incorrecto\n");
         return 1;
   }

   printf("Valor de la integral = %.12f\n", result);

   return 0;
}