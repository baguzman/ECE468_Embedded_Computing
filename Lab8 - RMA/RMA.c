// Brian Guzman
// ECE 4680
// Spring 2019
// Lab 8: RMA

#include <stdio.h>
#include <math.h>


int main()
{
	char *task_names[8];
	double period[8];
	double runtime[8];
	double totb[8];
	int i,l,k;
	double sum;
	
	//tasks
	task_names[0] = "Compute attitude data";
	task_names[1] = "Compute velocity data";
	task_names[2] = "Compose attitude message";
	task_names[3] = "Display data";
	task_names[4] = "Compose navigation message";
	task_names[5] = "Run-time built in test (BIT)";
	task_names[6] = "Compute position data";
	task_names[7] = "Compose test message";
	
	//period [ms]
	period[0] = 10.56;
	period[1] = 40.96;
	period[2] = 61.44;
	period[3] = 100.00;
	period[4] = 165.00;
	period[5] = 285.00;
	period[6] = 350.00;
	period[7] = 700.00;
	
	//runtimes [ms]
	runtime[0] = 1.30;
	runtime[1] = 4.70;
	runtime[2] = 9.00;
	runtime[3] = 23.00;
	runtime[4] = 38.3;
	runtime[5] = 10.00;
	runtime[6] = 3.00;
	runtime[7] = 2.00;
	
	//total blocking [ms]
	totb[0] = 3.30;
	totb[1] = 3.30;
	totb[2] = 9.30;
	totb[3] = 9.20;
	totb[4] = 5.20;
	totb[5] = 5.20;
	totb[6] = 2.00;
	totb[7] = 0.00;
	
	int count = 0;
	for(i = 0; i < 8; i++)
	{
		for(k = 0; k < i+1; k++)
		{
			for(l = 0; l < floor(period[i]/period[k]); l++)
			{
				count = 0;
				sum = 0; //reset sum and count
				
				
				while(count < i)
				{	
					sum += (runtime[count] + 0.153) * ceil(((l + 1)*period[k]) / period[count]);
					count++;
				}
				
				sum += runtime[i] + 0.153 + totb[i];
				
				if (sum < period[k]*(l+1))
				{
					printf("Task '%s' is schedulable at k = %d, l = %d.\n",task_names[i],(k+1),(l+1));
					break;
				}
			}
			if(l < floor(period[i]/period[k]))
			{
				break;
			}
		}
		if(k == (i+1))
		{
			printf("Task '%s' is not schedulable\n",task_names[i]);
		}
	}
	return 0;
}
	
