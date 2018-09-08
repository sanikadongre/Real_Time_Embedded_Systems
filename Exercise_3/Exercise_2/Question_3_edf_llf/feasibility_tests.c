#include <math.h>
#include <stdio.h>

#define TRUE 1
#define FALSE 0
#define U32_T unsigned int

// U=0.7333
U32_T ex0_period[] = {2, 10, 15};
U32_T ex0_wcet[] = {1, 1, 2};

// U=0.9857
U32_T ex1_period[] = {2, 5, 7};
U32_T ex1_wcet[] = {1, 1, 2};

// U=0.9967
U32_T ex2_period[] = {2, 5, 7, 13};
U32_T ex2_wcet[] = {1, 1, 1, 2};

// U=0.93
U32_T ex3_period[] = {3, 5, 15};
U32_T ex3_wcet[] = {1, 2, 3};

// U=1.0
U32_T ex4_period[] = {2, 4, 16};
U32_T ex4_wcet[] = {1, 1, 4};

U32_T ex5_period[] = {2, 5, 10};
U32_T ex5_wcet[] = {1, 2, 1};

U32_T ex6_period[] = {2, 5, 7, 13};
U32_T ex6_wcet[] = {1, 1, 1, 2};

U32_T ex7_period[] = {3, 5, 15};
U32_T ex7_wcet[] = {1, 2, 4};

U32_T ex8_period[] = {2, 5, 7, 13};
U32_T ex8_wcet[] = {1, 2, 1, 2};

U32_T ex9_period[] = {6, 8, 12, 24};
U32_T ex9_wcet[] = {1, 2, 2, 6};


void edf_feasibility(void)
{ 
 
    printf("Ex-0 U=%4.2f (C1=1, C2=1, C3=2; T1=2, T2=10, T3=15; T=D): ",
		   ((1.0/2.0) + (1.0/10.0) + (2.0/15.0)));
       float f = ((1.0/2.0) + (1.0/10.0) + (2.0/15.0));
        if(f <= 1)
        printf("FEASIBLE\n");
        else
        printf("INFEASIBLE\n");

    printf("Ex-1 U=%4.2f (C1=1, C2=1, C3=2; T1=2, T2=5, T3=7; T=D): ", 
		   ((1.0/2.0) + (1.0/5.0) + (2.0/7.0)));
        f = ((1.0/2.0) + (1.0/5.0) + (2.0/7.0));
        if(f <= 1)
        printf("FEASIBLE\n");
        else
        printf("INFEASIBLE\n");
	
    printf("Ex-2 U=%4.2f (C1=1, C2=1, C3=1, C4=2; T1=2, T2=5, T3=7, T4=13; T=D): ",
		   ((1.0/2.0) + (1.0/5.0) + (1.0/7.0) + (2.0/13.0)));
        f = ((1.0/2.0) + (1.0/5.0) + (1.0/7.0) + (2.0/13.0));
        if(f <= 1)
        printf("FEASIBLE\n");
        else
        printf("INFEASIBLE\n");
	
    printf("Ex-3 U=%4.2f (C1=1, C2=2, C3=3; T1=3, T2=5, T3=15; T=D): ",
		   ((1.0/3.0) + (2.0/5.0) + (3.0/15.0)));
	f = ((1.0/3.0) + (2.0/5.0) + (3.0/15.0));
        if(f <= 1)
        printf("FEASIBLE\n");
        else
        printf("INFEASIBLE\n");
	
    printf("Ex-4 U=%4.2f (C1=1, C2=1, C3=4; T1=2, T2=4, T3=16; T=D): ",
		   ((1.0/2.0) + (1.0/4.0) + (4.0/16.0)));
        f = ((1.0/2.0) + (1.0/4.0) + (4.0/16.0));
        if(f <= 1)
        printf("FEASIBLE\n");
        else
        printf("INFEASIBLE\n");
	
    printf("Ex-5 U=%4.2f (C1=1, C2=2, C3=1; T1=2, T2=5, T3=10; T=D): ",
		   ((1.0/2.0) + (2.0/5.0) + (1.0/10.0)));
        f = ((1.0/2.0) + (2.0/5.0) + (1.0/10.0));
        if(f <= 1)
        printf("FEASIBLE\n");
        else
        printf("INFEASIBLE\n");
	
    printf("Ex-6 U=%4.2f (C1=1, C2=1, C3=1, C4=2; T1=2, T2=5, T3=7, T4=13; T=D): ",
		   ((1.0/2.0) + (1.0/5.0) + (1.0/7.0) + (2.0/13.0)));
	f = ((1.0/2.0) + (1.0/5.0) + (1.0/7.0));
        if(f <= 1)
        printf("FEASIBLE\n");
        else
        printf("INFEASIBLE\n");
	
    printf("Ex-7 U=%4.2f (C1=1, C2=2, C3=4; T1=3, T2=5, T3=15; T=D): ",
		   ((1.0/3.0) + (2.0/5.0) + (4.0/15.0)));
	f = ((1.0/3.0) + (2.0/5.0) + (4.0/15.0));
        if(f <= 1)
        printf("FEASIBLE\n");
        else
        printf("INFEASIBLE\n");
	
    printf("Ex-8 U=%4.2f (C1=1, C2=2, C3=1, C4=2; T1=2, T2=5, T3=7, T4=13; T=D): ",
		   ((1.0/2.0) + (2.0/5.0) + (1.0/7.0) + (2.0/13.0)));
	f = ((1.0/2.0) + (2.0/5.0) + (1.0/7.0) + (2.0/13.0));
        if(f <= 1)
        printf("FEASIBLE\n");
        else
        printf("INFEASIBLE\n");
	
    printf("Ex-9 U=%4.2f (C1=1, C2=2, C3=2, C4=6; T1=6, T2=8, T3=12, T4=24; T=D): ",
		   ((1.0/6.0) + (2.0/8.0) + (2.0/12.0)+ (6.0/24.0)));
        f = ((1.0/6.0) + (2.0/8.0) + (2.0/12.0) + (6.0/24.0));
        if(f <= 1)
        printf("FEASIBLE\n");
        else
        printf("INFEASIBLE\n");
}

void llf_feasibility(void)
{ 
 
    printf("Ex-0 U=%4.2f (C1=1, C2=1, C3=2; T1=2, T2=10, T3=15; T=D): ",
		   ((1.0/2.0) + (1.0/10.0) + (2.0/15.0)));
        float f = ((1.0/2.0) + (1.0/10.0) + (2.0/15.0));
        if(f <= 1)
        printf("FEASIBLE\n");
        else
        printf("INFEASIBLE\n");

    printf("Ex-1 U=%4.2f (C1=1, C2=1, C3=2; T1=2, T2=5, T3=7; T=D): ", 
		   ((1.0/2.0) + (1.0/5.0) + (2.0/7.0)));
        f = ((1.0/2.0) + (1.0/5.0) + (2.0/7.0));
        if(f <= 1)
        printf("FEASIBLE\n");
        else
        printf("INFEASIBLE\n");
	
    printf("Ex-2 U=%4.2f (C1=1, C2=1, C3=1, C4=2; T1=2, T2=5, T3=7, T4=13; T=D): ",
		   ((1.0/2.0) + (1.0/5.0) + (1.0/7.0) + (2.0/13.0)));
        f = ((1.0/2.0) + (1.0/5.0) + (1.0/7.0) + (2.0/13.0));
        if(f <= 1)
        printf("FEASIBLE\n");
        else
        printf("INFEASIBLE\n");
	
    printf("Ex-3 U=%4.2f (C1=1, C2=2, C3=3; T1=3, T2=5, T3=15; T=D): ",
		   ((1.0/3.0) + (2.0/5.0) + (3.0/15.0)));
	f = ((1.0/3.0) + (2.0/5.0) + (3.0/15.0));
        if(f <= 1)
        printf("FEASIBLE\n");
        else
        printf("INFEASIBLE\n");
	
    printf("Ex-4 U=%4.2f (C1=1, C2=1, C3=4; T1=2, T2=4, T3=16; T=D): ",
		   ((1.0/2.0) + (1.0/4.0) + (4.0/16.0)));
        f = ((1.0/2.0) + (1.0/4.0) + (4.0/16.0));
        if(f <= 1)
        printf("FEASIBLE\n");
        else
        printf("INFEASIBLE\n");
	
    printf("Ex-5 U=%4.2f (C1=1, C2=2, C3=1; T1=2, T2=5, T3=10; T=D): ",
		   ((1.0/2.0) + (2.0/5.0) + (1.0/10.0)));
        f = ((1.0/2.0) + (2.0/5.0) + (1.0/10.0));
        if(f <= 1)
        printf("FEASIBLE\n");
        else
        printf("INFEASIBLE\n");
	
    printf("Ex-6 U=%4.2f (C1=1, C2=1, C3=1, C4=2; T1=2, T2=5, T3=7, T4=13; T=D): ",
		   ((1.0/2.0) + (1.0/5.0) + (1.0/7.0) + (2.0/13.0)));
	f = ((1.0/2.0) + (1.0/5.0) + (1.0/7.0));
        if(f <= 1)
        printf("FEASIBLE\n");
        else
        printf("INFEASIBLE\n");
	
    printf("Ex-7 U=%4.2f (C1=1, C2=2, C3=4; T1=3, T2=5, T3=15; T=D): ",
		   ((1.0/3.0) + (2.0/5.0) + (4.0/15.0)));
	f = ((1.0/3.0) + (2.0/5.0) + (4.0/15.0));
        if(f <= 1)
        printf("FEASIBLE\n");
        else
        printf("INFEASIBLE\n");
	
    printf("Ex-8 U=%4.2f (C1=1, C2=2, C3=1, C4=2; T1=2, T2=5, T3=7, T4=13; T=D): ",
		   ((1.0/2.0) + (2.0/5.0) + (1.0/7.0) + (2.0/13.0)));
        f = ((1.0/2.0) + (2.0/5.0) + (1.0/7.0) + (2.0/13.0));
        if(f <= 1)
        printf("FEASIBLE\n");
        else
        printf("INFEASIBLE\n");
	
    printf("Ex-9 U=%4.2f (C1=1, C2=2, C3=2, C4=6; T1=6, T2=8, T3=12, T4=24; T=D): ",
		   ((1.0/6.0) + (2.0/8.0) + (2.0/12.0)+ (6.0/24.0)));
	f = ((1.0/6.0) + (2.0/8.0) + (2.0/12.0) + (6.0/24.0));
        if(f <= 1)
        printf("FEASIBLE\n");
        else
        printf("INFEASIBLE\n");
}

void main(void)
{
  printf("EDF Feasibility: \n");
  edf_feasibility();
  printf("LLF Feasibility: \n");
  llf_feasibility();
}
