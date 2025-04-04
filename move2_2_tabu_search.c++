#include <bits/stdc++.h>
#include <fstream>
#include <vector>
#include <array>
#include <stdlib.h> 
#include <math.h>
#include<stdbool.h>
#include<chrono>
#include<iostream>
using namespace std;
using namespace std:: chrono;
vector<array<double,3>> numbers;
vector<int>op_sol;
float op_fit;
vector<int>current_sol;
vector<array<int,4>>tabu_list;//tabu_list save 4 nodes of 2 exchange edges : (x,y,z,t)-> edge(x,y) and edge(z,t)
int tabu_tenure;// the length of tabu list
float current_fit;
int ite_count =0;
void write_result(string output,float duration,string filename, double op_result)
{
    ofstream file(output,ios::app);
    file<< filename<<endl;
    file<<"optimum solution found:"<<"\n";
    for (int i=0;i<op_sol.size();i++)file<<op_sol[i]<<"\t";
    file<<endl;
    file<<"route = "<<1/op_fit<<endl;
    file<<" Gap = "<<(1/(op_fit*op_result)-1)*100<<"%"<<endl;
    file<<"running time = "<<duration/pow(10,6)<<"seconds";
    file<<endl<<endl;
    file.close();
}
vector<string> readfilename(string  & source)
{
    ifstream file(source);
    string file_name;
    vector<string> file_name_list;
    while (file>>file_name)
    {
        file_name_list.push_back(file_name);
    }
    file.close();
    return file_name_list;
}
vector<double> readopresult(string op_result_file)
{
    ifstream file(op_result_file);
    vector <double> op_result;
    double num;
    int point=0;
    if (!file)cout << "Unable to open file";
    else
    {
        while (file >> num)
        {
            op_result.push_back(num);
        }
    }
    file.close();
    return op_result;
}
void readDoublesFromFile(string & filename)
{
    ifstream file(filename);
    array<double,3> num_arr;// save information about location of a inserted note
    double num;
    if (!file)cout << "Unable to open file";
    else
    {
        while (file >> num)
        {
            num_arr[0]=num;
            file>>num_arr[1];
            file>>num_arr[2];
            numbers.push_back(num_arr);
        }
    }
    file.close();
}
double fitness(double**distance,vector<int> sol)
{
    double route = 0;
    for (int i=0;i<sol.size()-1;i++)route+=distance[sol[i]][sol[i+1]];
    route += distance[sol[sol.size()-1]][sol[0]];
    return 1/route;
}
vector<int> greedy_initialization(double**distance)
{
    vector<int> ini_sol;
    ini_sol.push_back(0);
    int count=1;
    vector<int>ele_left;
    for (int i=1;i<numbers.size();i++)ele_left.push_back(i);
    while (!ele_left.empty())
    {
        int min_route=distance[ini_sol.back()][ele_left[0]];
        int next=0;
        for (int i=1;i<ele_left.size();i++)
        {
            if (distance[ini_sol.back()][ele_left[i]]<min_route)
            {
                min_route=distance[ini_sol.back()][ele_left[i]];
                next=i;
            }
        }
        ini_sol.push_back(ele_left[next]);
        ele_left.erase(ele_left.begin()+next);
    }
    return ini_sol;
}
vector<int>sol_change(vector<int>current_sol,int i,int j)
{
    vector<int>new_sol=current_sol;// save the current processing 
    if (j==current_sol.size()-1)// j=current_sol,size-1 indicate the second exchange edge: current_sol[n-1]-current_sol[0]
    {
        int a,b;
        a=new_sol[i];
        b=new_sol[i+1];
        new_sol[i]=new_sol[current_sol.size()-1];
        new_sol[i+1]=new_sol[0];
        new_sol[current_sol.size()-1]=a;
        new_sol[0]=b;
    }
    else
    {
        int a,b;
        a=new_sol[i];
        b=new_sol[i+1];
        new_sol[i]=new_sol[j];
        new_sol[i+1]=new_sol[j+1];
        new_sol[j]=a;
        new_sol[j+1]=b;
    }
    return new_sol;
}
bool tabu_check(int i,int j)
{
    bool tabu=false;
    for (int k=0;k<tabu_list.size()&&!tabu;k++)
    {
        if (j<numbers.size()-1)tabu=(tabu_list[k][0]==current_sol[i] &&tabu_list[k][1]==current_sol[i+1]&&tabu_list[k][2]==current_sol[j] &&tabu_list[k][3]==current_sol[j+1])||(tabu_list[k][2]==current_sol[i] &&tabu_list[k][3]==current_sol[i+1]&&tabu_list[k][0]==current_sol[j] &&tabu_list[k][1]==current_sol[j+1]);
        else tabu=(tabu_list[k][0]==current_sol[i] &&tabu_list[k][1]==current_sol[i+1]&&tabu_list[k][2]==current_sol[j] &&tabu_list[k][3]==current_sol[0])||(tabu_list[k][2]==current_sol[i] &&tabu_list[k][3]==current_sol[i+1]&&tabu_list[k][0]==current_sol[j] &&tabu_list[k][1]==current_sol[0]);
    }//tabu=true mean there is a tabu move like the considering move 
    return tabu;
}
bool tabu_check2(int i,int j)
{
    bool tabu;
    tabu=(tabu_list[tabu_list.size()-1][0]==current_sol[i] &&tabu_list[tabu_list.size()-1][1]==current_sol[i+1]&&tabu_list[tabu_list.size()-1][2]==current_sol[j] &&tabu_list[tabu_list.size()-1][3]==current_sol[j+1])||(tabu_list[tabu_list.size()-1][2]==current_sol[i] &&tabu_list[tabu_list.size()-1][3]==current_sol[i+1]&&tabu_list[tabu_list.size()-1][0]==current_sol[j] &&tabu_list[tabu_list.size()-1][1]==current_sol[j+1]);
    return !tabu;
}
void tabu_process(double**distance)
{
    vector<int> process_sol;
    int m,n;
    process_sol=current_sol;
    float best_fitness=0;
    for (int i=0;i<numbers.size()-2;i++)
    {
        for (int j=i+2;j<numbers.size();j++)
        {
            process_sol=sol_change(current_sol,i,j);
            if(fitness(distance,process_sol)>best_fitness)
            {
                best_fitness=fitness(distance,process_sol);
                m=i;
                n=j;
            }
        }
    }
    if (!tabu_check(m,n)||((fitness(distance,sol_change(current_sol,m,n))-op_fit>pow(10,-7))&&tabu_check2(m,n))||((fitness(distance,sol_change(current_sol,m,n))>1.2*current_fit)&&tabu_check2(m,n)))
    {
        current_sol = sol_change(current_sol,m,n);
        current_fit=fitness(distance,current_sol);
        array<int,4>a;
        a[0]=current_sol[m];
        a[1]=current_sol[m+1];
        a[2]=current_sol[n];
        if (n<numbers.size()-1)a[3]=current_sol[n+1];
        else a[3]=current_sol[0];
        tabu_list.push_back(a);
        if (tabu_list.size()>tabu_tenure)tabu_list.erase(tabu_list.begin());
        if (current_fit>op_fit)
        {
            op_fit=current_fit;
            op_sol=current_sol;
            ite_count=0;
        }
        else ite_count++;
    }
    else
    {
        double max_fit=fitness(distance,sol_change(current_sol,m,n));//max_fit save the smallest fit considered; the subsequent fit value must lower due to the value is considered from large to small.
        vector<array<int,2>> wr;//save the considered moves
        array<int,2>a;
        a[0]=m;
        a[1]=n;
        wr.push_back(a);
        bool mark=true;//used for break out immmediately from while loop
        while (mark)
        {
            best_fitness=0;//best_fitness saves the value of the best considered moves inside a "while" loop
            bool remark=true;
            for (int i=0;(i<numbers.size()-2)&&remark;i++)
            {
                for (int j=i+2;(j<numbers.size())&&remark;j++)
                {
                    process_sol=sol_change(current_sol,i,j);
                    if((fitness(distance,process_sol)>best_fitness)&&(max_fit-fitness(distance,process_sol)>pow(10,-8)))
                    {
                        best_fitness=fitness(distance,process_sol);
                        m=i;
                        n=j;
                    }
                    else
                    {
                        if(fitness(distance,process_sol)==max_fit)
                        {
                            for (int k=0;k<wr.size();k++)
                            {
                                remark=(i==wr[k][0]&&j==wr[k][1]); //check a move if it is inside the wr
                                //remark= true if found
                                if (remark) break;
                            }
                            if(!remark)//remark=false if  considering move is not in wr yet
                            {
                                best_fitness=max_fit;
                                m=i;
                                n=j;
                            }
                        }
                    }
                }
            }
            //cout<<tabu_check(m,n)<<endl;
            //cout<<fitness(distance,sol_change(current_sol,m,n))<<"\t"<<op_fit<<"\t"<<current_fit<<endl;
            if (!tabu_check(m,n)||fitness(distance,sol_change(current_sol,m,n))-op_fit>pow(10,-7)||fitness(distance,sol_change(current_sol,m,n))>1.2*current_fit)
            {
                current_sol = sol_change(current_sol,m,n);
                current_fit=fitness(distance,current_sol);
                array<int,4>a;
                a[0]=current_sol[m];
                a[1]=current_sol[m+1];
                a[2]=current_sol[n];
                if (n<numbers.size()-1)a[3]=current_sol[n+1];
                else a[3]=current_sol[0];
                tabu_list.push_back(a);
                if (tabu_list.size()>tabu_tenure)tabu_list.erase(tabu_list.begin());
                if (current_fit>op_fit)
                {
                    op_sol=current_sol;
                    op_fit=current_fit;
                    ite_count=0;
                }
                else ite_count++;
                mark=false;
            }
            else
            {
                a[0]=m;
                a[1]=n;
                wr.push_back(a);
                max_fit=best_fitness;
            }
        }
    }
}
void distance_computing(double**distance)
{
    for (int i=0;i<numbers.size();i++)
    {
        for (int j=0;j<numbers.size();j++)distance[i][j]=sqrt(pow(numbers[i][1]-numbers[j][1],2)+pow(numbers[i][2]-numbers[j][2],2));
    }
}
int main() 
{
    string source="source.txt";
    string output="output1.txt";
    string op_result_file="op.txt";
    //cout<<"Input the source file name: ";
    //cin>>source;
    //cout<<"Input the output file: ";
    //cin>>output;
    //cout<<"Input the optimun result file: ";
    //cin>>op_result_file;
    vector<double>op_result;
    vector<string> file_name_list;
    file_name_list= readfilename(source);
    op_result=readopresult(op_result_file);
    while (!file_name_list.empty())
    {

        string filename;
        filename= file_name_list[0];
        file_name_list.erase(file_name_list.begin());
        auto start = high_resolution_clock::now();
        readDoublesFromFile(filename);
        double**distance=(double**)malloc(numbers.size()*sizeof(double*));
        for (int i=0;i<numbers.size();i++)
        {
            distance[i]=(double*)malloc(numbers.size()*sizeof(double));
        }
        distance_computing(distance);
        op_sol=greedy_initialization(distance);
        current_sol=op_sol;
        op_fit=current_fit=fitness(distance,current_sol);
        tabu_tenure=(int)sqrt(numbers.size());
        while (ite_count < 200)
        {
            tabu_process(distance);
        }
        for (int i=0;i<numbers.size();i++)free(distance[i]);
        free(distance);
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end - start);
        float time=duration.count();
        write_result(output,time,filename,op_result[0]);
        op_result.erase(op_result.begin());
        numbers.clear();
        op_sol.clear();
        op_fit=0;
        current_sol.clear();
        tabu_list.clear();
        current_fit=0;
        ite_count =0;
    }
    return 0;
}