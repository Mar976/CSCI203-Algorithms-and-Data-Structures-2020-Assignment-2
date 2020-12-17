//======================================================
//ASSIGNMENT 2
//TITLE: SIMULATES THE QUEUING AND SERVICE OF AIRLINE PASSENGERS
//AUTHOR: MARZIA ABDUL RAHIM
//ID: MAR976-6287645
//======================================================

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
using namespace std;

#define MAX(a,b) (a>b?a:b)

//STRUCT 
struct Server
{
	int id=-1;
    double finishTime=0,totalIdleTime=0,totalServiceTime=0;
};
struct Customer
{
	double arrivalTime=0,waitTime=0,serviceDurtation=0,queueTime=0;
    int type=-1;
	Server server;
	bool bussinessServer=false;
};
struct Event
{
	int type=-1;
	double eventTime=0;
	Customer customer;
};
//GLOBAL VARIABLES
//ARRAY AND INDEX	
Event *EventHeap;										//EVENT HEAP
int indexH=0;
Server *idleServerB;									//IDLE BUSSINESS SERVER STACK
int IdleServerBusIndex=0;
Server *idleServerT;									//IDLE TOURIST SERVER STACK
int IdleServerTourIndex=0;
Event BussinessQueue[500];								//WAITING BUSSINESS SERVER QUEUE
int custBusFront=0;
int custBusRear=0;
Event TouristQueue[500];								//WAITING TOURIST SERVER QUEUE
int custTourFront=0;	
int custTourRear=0;
int ServerNumT=0, ServerNumB=0;
int sizeE=0;
//STATISTICS->TO KEEP TRACK OF THE AVERAGE, TOTAL AND OTHER STATISTICS 
int totalCustomerServed=0, totalBussinessCustomer=0, totalTouristCustomer=0, totalCustomer=0,maxQueueLengthT=0,maxQueueLengthB=0;
double totalQueueLengthCountT=0,totalQueueLengthT=0,totalQueueLengthCountB=0,totalQueueLengthB=0;
double lastEventTime=0,totalServiceTimeT=0,totalServiceTimeB=0,totalQueueTimeT=0,totalQueueTimeB=0,oldTimeT=0,oldTimeB=0;
//
int customerArrival=0;									//IF CUSTOMER IS ARRIVAL THEN IT IS TYPE ZERO
int serviceCompletion=1;								//IF IT IS SERVICE COMPLETION THEN IT IS TYPE ONE
int touristType=0;										//IF THE CUSTOMER IS TOURIST THEN IT IS ZERO
int bussinessType=1;									//IF CUSTOMER IS BUSSINESS THEN IT IS ONE

//FUNCTION PROTOTYPES
void printStatistics(int);
int numberOfEachClass(int);
//HEAP
void addServer(Customer &c, double,double,int);
void addEvent(int,double,Customer &customer);
Event getNextEvent();
void MinHeapify(int);
int parent(int i);
int left(int i);
int right(int i);
//STACK
bool EmptyServerStack(int);
bool FullServerStack(int);
Server pop(int);
void push(Server,int);
//QUEUE
bool EmptyCustomerQueue(int);
bool FullCustomerQueue(int);
void Enqueue(Event);
Event Dequeue(int); 

int main()
{
	//GETTING FILE NAME
	string fileName="";
	cout<<"FILE NAME: ";
	cin>>fileName;
	//============================================================================================
	//===========================================PASS 1===========================================
	//============================================================================================
	//OPENING THE FILE 
	ifstream inData;
	inData.open(fileName);
	if(!inData.good()) {
		cerr<<"Can not open the file!"<<endl;
		return 1;
	}
	else {
		//INITIALIZE
		inData>>ServerNumB>>ServerNumT;
		sizeE=ServerNumB+ServerNumT+1;
		//DYNAMIC MEMORY ALLOCATION
		EventHeap=new Event[sizeE];		
		idleServerB=new Server[ServerNumB];
		idleServerT=new Server[ServerNumT];
		//SET THE DEFAULT VALUES
		for(int i=0;i<ServerNumB;i++){
			Server server;
			server.id=i+1;
			push(server,1);
		}
		for(int i=0;i<ServerNumT;i++){
			Server server;
			server.id=i+1;
			push(server,0);
		}
		//READING THE FIRST CUSTOMER FROM THE FILE 
		bool endOfFile=false;
		Customer customer;
		inData>>customer.arrivalTime>>customer.type>>customer.serviceDurtation;
		addEvent(customerArrival,customer.arrivalTime,customer);
		totalCustomer++;
		//MAIN LOOP
		while(indexH!=0){
			Event next=getNextEvent();
			//cout<<next.eventTime<<endl;
			//1.HANDLE CUSTOMER ARRIVAL EVENT
			if(next.type==customerArrival) {
				//IF FREE SERVER THEN USE IT 
				if(!EmptyServerStack(next.customer.type)) {
					double finishTime = next.customer.arrivalTime+next.customer.serviceDurtation;
					addServer(next.customer, next.eventTime, finishTime,next.customer.type);
					//ADDING IT BACK TO THE HEAP
					addEvent(serviceCompletion,finishTime,next.customer);
				}
				else {
					next.customer.queueTime=next.eventTime;
					//ADD IT TO THE RIGHT QUEUE
					Enqueue(next);
				}

				//NOW READ A NEW CUSTOMER FROM THE FILE
				if(!endOfFile){
					Customer customer;
					inData>>customer.arrivalTime>>customer.type>>customer.serviceDurtation;
					if(customer.arrivalTime==0 && customer.type==0 && customer.serviceDurtation==0)	{
						endOfFile=true;
						inData.close();
					}
					else{
						//FOR STATISTIC
						totalCustomer++;																//TOTAL NUMBER OF CUSTOMERS
						customer.waitTime=0;
						//ADD IT TO THE HEAP AS ARRIVAL EVENT
						addEvent(customerArrival,next.customer.arrivalTime,customer);
					}
				}
			}

			//2. HANDLE SERVER EVENT 
			else {
				int type=next.customer.type;
				//FOR STATISTIC
				totalCustomerServed++;
				if(type==0){
					totalServiceTimeT+=next.customer.serviceDurtation+next.customer.waitTime;			//TOTAL SERVICED TIME->TOURIST SERVER
				}
				else if(type==1){
					totalServiceTimeB+=next.customer.serviceDurtation+next.customer.waitTime;			//TOTAL SERVICED TIME->BUSINESS SERVER
				}
				//MAKE THE SERVER AVAILABLE 
				push(next.customer.server,type);
				//ASSIGN SERVER TO EVENT WAITING IN THE QUEUEU OF THE SAME TYPE
				if(!EmptyCustomerQueue(type)) {
					//GET THE NEXT CUSTOMER
					Event nextEvent=Dequeue(type);
					nextEvent.customer.waitTime=next.eventTime-nextEvent.customer.queueTime;
					double finishTime=next.eventTime+nextEvent.customer.serviceDurtation;
					//ADDING THE SERVER TO THE EVENT
					next.customer.server.totalServiceTime += (finishTime - next.eventTime);
					next.customer.server.totalIdleTime += next.eventTime-next.customer.server.finishTime;
					next.customer.server.finishTime = finishTime;
					nextEvent.customer.server=next.customer.server;
					pop(type);
					addEvent(serviceCompletion,finishTime,nextEvent.customer);
					//FOR STATISTIC
					if(nextEvent.customer.type==0){
						totalQueueTimeT+=next.customer.waitTime;										//TOTAL QUEUE TIME->TOURIST SERVER
					}
					else if(nextEvent.customer.type==1){
						totalQueueTimeB+=next.customer.waitTime;										//TOTAL QUEUE TIME->BUSSINESS SERVER
					}
				}
				if(indexH==0) {
					lastEventTime=next.eventTime;
				}
			}
		}
		
	}
	//PRINT
	printStatistics(1);
	//============================================================================================
	//===========================================PASS 2===========================================
	//============================================================================================
	//RESET THE VARIABLES 
	//INDEX
	indexH=IdleServerBusIndex=IdleServerTourIndex=custBusFront=custBusRear=custTourFront=custTourRear=0;
	//STATISTIC
	totalCustomerServed=totalBussinessCustomer=totalTouristCustomer=totalCustomer=maxQueueLengthT=maxQueueLengthB=0;
	totalQueueLengthCountT=totalQueueLengthT=totalQueueLengthCountB=totalQueueLengthB=0;
	lastEventTime=totalServiceTimeT=totalServiceTimeB=totalQueueTimeT=totalQueueTimeB=0;
	oldTimeT=oldTimeB=0;
	
	//OPENING THE FILE
	inData.open(fileName);
	if(!inData.good()) {
		cerr<<"Can not open the file!"<<endl;
		return 1;
	}
	else {
		//INITIALIZE
		inData>>ServerNumB>>ServerNumT;
		sizeE=ServerNumB+ServerNumT+1;
		//DYNAMIC MEMORY ALLOCATION
		EventHeap=new Event[sizeE];		
		idleServerB=new Server[ServerNumB];
		idleServerT=new Server[ServerNumT];
		//SET THE DEFAULT VALUES
		//SET THE DEFAULT VALUES FOR THE SERVERS
		for(int i=0;i<ServerNumB;i++){
			Server server;
			server.id=i+1;
			server.finishTime=server.totalIdleTime=server.totalServiceTime=0;
			push(server,1);
		}
		for(int i=0;i<ServerNumT;i++){
			Server server;
			server.id=i+1;
			server.finishTime=server.totalIdleTime=server.totalServiceTime=0;
			push(server,0);
		}
		//READING THE FIRST CUSTOMER FROM THE FILE 
		bool endOfFile=false;
		Customer customer;
		inData>>customer.arrivalTime>>customer.type>>customer.serviceDurtation;
		addEvent(customerArrival,customer.arrivalTime,customer);
		totalCustomer++;
		//MAIN LOOP
		while(indexH!=0){
			Event next=getNextEvent();
			//1.HANDLE CUSTOMER ARRIVAL EVENT
			if(next.type==customerArrival) {
				//CHECK FOR AVAILIBALITY OF SERVER
				//IF SERVER OF THE RIGHT TYPE IS FREE THEN USE THAT 
				if(!EmptyServerStack(next.customer.type) || !EmptyServerStack(bussinessType)){
					double finishTime = next.customer.arrivalTime+next.customer.serviceDurtation;
					if(!EmptyServerStack(next.customer.type)){
						addServer(next.customer, next.eventTime, finishTime,next.customer.type);
					}
					//OTHERWISE CHECK IF ANY BUSSINESSS SERVER IS FREE TO SERVER TOURIST CUSTOMER
					else if(!EmptyServerStack(bussinessType))
					{
						next.customer.bussinessServer=true;
						addServer(next.customer, next.eventTime, finishTime,bussinessType);
					}
					//ADDING IT BACK TO THE HEAP
					addEvent(serviceCompletion,finishTime,next.customer);
				}
				else {
					next.customer.queueTime=next.eventTime;
					//ADD IT TO THE RIGHT QUEUE
					Enqueue(next);
				}

				//NOW READ A NEW CUSTOMER FROM THE FILE
				if(!endOfFile){
					Customer customer;
					inData>>customer.arrivalTime>>customer.type>>customer.serviceDurtation;
					if(customer.arrivalTime==0 && customer.type==0 && customer.serviceDurtation==0)	{
						endOfFile=true;
						inData.close();
					}
					else{
						//FOR STATISTIC
						totalCustomer++;																//TOTAL NUMBER OF CUSTOMERS
						customer.waitTime=0;
						//ADD IT TO THE HEAP AS ARRIVAL EVENT
						addEvent(customerArrival,next.customer.arrivalTime,customer);
					}
				}
			}
			//2. HANDLE SERVER EVENT 
			else {
				//FOR STATISTIC
				totalCustomerServed++;
				if(next.customer.type==0 && !next.customer.bussinessServer){
					totalServiceTimeT+=next.customer.serviceDurtation+next.customer.waitTime;			//TOTAL SERVICED TIME->TOURIST SERVER
				}
				else if(next.customer.type==1 || next.customer.bussinessServer){
					totalServiceTimeB+=next.customer.serviceDurtation+next.customer.waitTime;			//TOTAL SERVICED TIME->BUSINESS SERVER
				}
				//MAKE THE SERVER AVAILABLE
				if(next.customer.bussinessServer){
					push(next.customer.server,bussinessType);
				} 
				else {
					push(next.customer.server,next.customer.type);
				}
				
				
				//ASSIGN SERVER TO EVENT WAITING IN THE QUEUEU OF THE SAME TYPE (SAME TYPES)
				if(!EmptyCustomerQueue(next.customer.type) && !EmptyServerStack(next.customer.type)) {
					//GET THE NEXT CUSTOMER
					Event nextEvent=Dequeue(next.customer.type);
					nextEvent.customer.waitTime=next.eventTime-nextEvent.customer.queueTime;
					double finishTime=next.eventTime+nextEvent.customer.serviceDurtation;
					//ADDING THE SERVER TO THE EVENT
					next.customer.server.totalServiceTime += (finishTime - next.eventTime);
					next.customer.server.totalIdleTime += next.eventTime-next.customer.server.finishTime;
					next.customer.server.finishTime = finishTime;
					nextEvent.customer.server=next.customer.server;
					pop(next.customer.type);
					addEvent(serviceCompletion,finishTime,nextEvent.customer);
					//FOR STATISTIC
					if(nextEvent.customer.type==0){
						totalQueueTimeT+=next.customer.waitTime;										//TOTAL QUEUE TIME->TOURIST SERVER
					}
					else if(nextEvent.customer.type==1){
						totalQueueTimeB+=next.customer.waitTime;										//TOTAL QUEUE TIME->BUSSINESS SERVER
					}
				}
				//CHECK IF ANY BUSSINESSS SERVER IS FREE TO SERVE TOURIST EVENT 
				else if(!EmptyCustomerQueue(touristType) && !EmptyServerStack(bussinessType)){
					//GET THE NEXT CUSTOMER
					Event nextEvent=Dequeue(touristType);
					nextEvent.customer.bussinessServer=true;
					nextEvent.customer.waitTime=next.eventTime-nextEvent.customer.queueTime;
					double finishTime=next.eventTime+nextEvent.customer.serviceDurtation;
					//ADDING THE SERVER TO THE EVENT
					next.customer.server.totalServiceTime += (finishTime - next.eventTime);
					next.customer.server.totalIdleTime += next.eventTime-next.customer.server.finishTime;
					next.customer.server.finishTime = finishTime;
					nextEvent.customer.server=next.customer.server;
					pop(bussinessType);
					addEvent(serviceCompletion,finishTime,nextEvent.customer);
					//FOR STATISTIC
					totalQueueTimeT+=next.customer.waitTime;
				}
				if(indexH==0) {
					lastEventTime=next.eventTime;
				}
			}
		}
	}
	
	//PRINT
	printStatistics(2);

	delete[] EventHeap;
	delete[] idleServerB;
	delete[] idleServerT;
	return 0;
}
//===========================================HEAP===========================================
Event getNextEvent(){
	if (indexH <= 0){
		cerr<<"EVENT IS EMPTY"<<endl;
	}
    if (indexH == 1) {
        indexH--;
        return EventHeap[0];
    }

    Event next = EventHeap[0];
    EventHeap[0] = EventHeap[indexH - 1];
    indexH--;
    MinHeapify(0);
    return next;
}
void addServer(Customer &customer, double startTime,double finishTime,int customerType){
    if (!EmptyServerStack(customerType)) {
		Server server=pop(customerType);
        server.totalServiceTime += (finishTime - startTime);
        server.totalIdleTime += startTime-server.finishTime;
        server.finishTime = finishTime;
		customer.server= server;
    }
}
//SOURCE:https://www.geeksforgeeks.org/binary-heap/
void addEvent(int eventType, double eventTime, Customer &customer){	
    if (indexH == sizeE) {
        cerr<<"EVENT IS FULL"<<endl;
		return;
    }

	if (indexH == (sizeE / 2)) {
        Event *temp = EventHeap;
        EventHeap = new Event[sizeE * 2];
        int i;
        for (i = 0; i < sizeE; i++) { EventHeap[i] = temp[i]; }
        delete[] temp;
    }

    int i = indexH++;
	Event event;
	event.type=eventType;
	event.eventTime=eventTime;
	event.customer=customer;
    EventHeap[i]=event;

    //FIX THE HEAP PEROPERTY IF IT IS VIOLATED 
    while (i != 0 && EventHeap[parent(i)].eventTime > EventHeap[i].eventTime) {
        swap(EventHeap[i], EventHeap[parent(i)]);
        i = parent(i);
    }
}
void MinHeapify(int i){
	int smallest = i;

    if (left(i) <= indexH && EventHeap[left(i)].eventTime < EventHeap[i].eventTime)
        smallest = left(i);

    if (right(i) <= indexH && EventHeap[right(i)].eventTime < EventHeap[smallest].eventTime)
        smallest = right(i);

    if (smallest != i) {
        swap(EventHeap[i], EventHeap[smallest]);
        MinHeapify(smallest);
    }
}
int parent(int i){ return (i-1)/2;} 
int left(int i) {return (i*2)+1;}
int right(int i) {return (i*2)+2;}
//===========================================STACK===========================================
bool EmptyServerStack(int typeEvent){
	if(typeEvent==touristType) {
		return IdleServerTourIndex==0;
	}
	if(typeEvent==bussinessType) {
		return IdleServerBusIndex==0;
	}
}
bool FullServerStack(int typeEvent){
	if(typeEvent==touristType) {
		return IdleServerTourIndex==ServerNumT;
	}
	if(typeEvent==bussinessType) {
		return IdleServerBusIndex==ServerNumB;
	}
}
Server pop(int typeEvent)
{
	Server temp;
	if(EmptyServerStack(typeEvent)) {
		cerr<<"STACK IS EMPTY!"<<endl;
	}
	if(typeEvent==0) {
		if(IdleServerTourIndex!=0) {
			totalTouristCustomer++;
			temp= idleServerT[--IdleServerTourIndex];
		}
	}
	if(typeEvent==1) {
		if(IdleServerBusIndex!=0) {
			totalBussinessCustomer++;
			temp= idleServerB[--IdleServerBusIndex];
		}
	}
	return temp;
}
void push(Server serverIn,int typeIn){
	if(FullServerStack(typeIn)) {
		cerr<<"STACK IS FULL!"<<endl;
	}
	else if(typeIn==0) {
			idleServerT[IdleServerTourIndex++]=serverIn;
	}
	else if(typeIn==1) {
			idleServerB[IdleServerBusIndex++]=serverIn;
	}
}
//===========================================QUEUE FIFO===========================================
//CHECK IF QUEUE IS EMPTY
bool EmptyCustomerQueue(int customerType){
	if(customerType==0) {
		return custTourFront==custTourRear;
	}
	if(customerType==1) {
		return custBusFront==custBusRear;
	}
}
//CHECK IF QUEUE IS FILL
bool FullCustomerQueue(int customerType){
	if(customerType==0) {
		return 500==custTourRear;
	}
	if(customerType==1) {
		return 500==custBusRear;
	}
}
void Enqueue(Event event) { 
	if(FullCustomerQueue(event.customer.type)) {
		cerr<<"QUEUE IS FULL!"<<endl; 
    } 
	else {
		if(event.customer.type==touristType){
			//INSERT ELEMENT IN THE REAR OF TOURIST 
			TouristQueue[custTourRear] = event; 
			custTourRear++;
			//FOR STATISTIC
			maxQueueLengthT=MAX(maxQueueLengthT,custTourRear);				//GETTING MAX LENGTH
			totalQueueLengthT+=(event.eventTime-oldTimeT)*custTourRear;								//GETTING TOTAL OF LENGTH 
			oldTimeT=event.eventTime;
			
		}
		else
		{
			//INSERT ELEMENT IN THE REAR OF BUSSINESS
			BussinessQueue[custBusRear] = event; 
			custBusRear++;
			//FOR STATISTIC
			maxQueueLengthB=MAX(maxQueueLengthB,custBusRear);				//GETTING MAX LENGTH
			totalQueueLengthB+=(event.eventTime-oldTimeB)*custBusRear;						//GETTING TOTAL OF LENGTH 
			oldTimeB=event.eventTime;
		}
	}
}
//RETURN AN ELEMENT FROM THE FRONT OF THE QUEUE
Event Dequeue(int eventType) { 
	Event temp;
	if(EmptyCustomerQueue(eventType)) {
		cerr<<"QUEUE IS EMPTY!"<<endl;
    } 
    //SHIFT ALL THE ELEMENT TO THE LEFT BY 1
    else if(eventType==touristType) {
        temp=TouristQueue[0];
        for (int i = 0; i < custTourRear - 1; i++) { 
            TouristQueue[i] = TouristQueue[i + 1]; 
        } 
        custTourRear--; 
    }
    else {
        temp=BussinessQueue[0];
        for (int i = 0; i < custBusRear - 1; i++) { 
            BussinessQueue[i] = BussinessQueue[i + 1]; 
        } 
        custBusRear--; 
    }
    return temp; 
}
//===========================================PRINT===========================================
void printStatistics(int passNum)
{
	cout<<"Pass "<<passNum<<": Business servers exclusively serve business class"<<endl;
	cout<<left << setw(70)<<"\nNumber of people served:"<<totalCustomerServed<<endl;
	cout<<left << setw(70)<<"Time last service is completed:"<<setprecision(2)<<fixed<<lastEventTime<<endl;

	cout<<left << setw(72)<<"\nBusiness class customers:"<<totalBussinessCustomer<<endl;
	cout<<left << setw(70)<<"Average total service time:"<<totalServiceTimeB/totalBussinessCustomer<<endl;
	cout<<left << setw(70)<<"Average total time in queue:"<<totalQueueTimeB/totalBussinessCustomer<<endl;
	cout<<left << setw(70)<<"Ave length of queue:"<<totalQueueLengthB/lastEventTime<<endl;
	cout<<left << setw(70)<<"Maximum number queued:"<<maxQueueLengthB<<endl;

	cout<<left << setw(72)<<"\nTourist class customers:"<<totalTouristCustomer<<endl;
	cout<<left << setw(70)<<"Average total service time:"<<totalServiceTimeT/totalTouristCustomer<<endl;
	cout<<left << setw(70)<<"Average total time in queue:"<<totalQueueTimeT/totalTouristCustomer<<endl;
	cout<<left << setw(70)<<"Ave length of queue:"<<totalQueueLengthT/lastEventTime<<endl;
	cout<<left << setw(70)<<"Maximum number queued:"<<maxQueueLengthT<<endl;

	cout<<left << setw(72)<<"\nAll customers:"<<totalCustomer<<endl;
	cout<<left << setw(70)<<"Average total service time:"<<(totalServiceTimeT+totalServiceTimeB)/totalCustomerServed<<endl;
	cout<<left << setw(70)<<"Average total time in queue:"<<(totalQueueTimeT+totalQueueTimeB)/totalCustomerServed<<endl;
	cout<<left << setw(70)<<"Ave length of queue:"<<(totalQueueLengthT+totalQueueLengthB)/lastEventTime<<endl;
	cout<<left << setw(70)<<"Maximum number queues:"<<MAX(maxQueueLengthB,maxQueueLengthT)<<endl;

	cout<<"\nBusiness class servers:"<<endl;
	for(int i=0;i<ServerNumB;i++){
		idleServerB[i].totalIdleTime += lastEventTime - idleServerB[i].finishTime;
		cout<<"Total idle time for business class server "<<i+1<<left << setw(25)<<":"<<idleServerB[i].totalIdleTime<<endl;
	}
	cout<<"\nTourist class servers:"<<endl;
	for(int i=0;i<ServerNumT;i++){
		idleServerT[i].totalIdleTime += lastEventTime - idleServerT[i].finishTime;
		cout<<"Total idle time for tourist class server "<<i+1<<left << setw(27)<<":"<<idleServerT[i].totalIdleTime<<endl;
	}
	cout<<endl;
}