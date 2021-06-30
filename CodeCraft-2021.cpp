#ifndef INCLUDE_SERVER
#define INCLUDE_SERVER
// #define TEST
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include "queue"
#include <time.h>
#include <assert.h>
#include <map>
#include <list>

using namespace std;


class ZeroCpuHost{
public:
    int node; //1A2B;
    int hostID;
    int p_level; //优先级;
    string hostName;
    vector<int> hostinfo;
    vector<int> unSource;//CPU MEM服务器剩余的资源
    map<string, vector<int>> vmName_info;
};
class ZeroMemHost{
public:
    int node; //1A2B;
    int hostID;
    int p_level; //优先级;
    string hostName;
    vector<int> hostinfo;//CPU MEM
    vector<int> unSource;//CPU MEM服务器剩余的资源
    map<string, vector<int>> vmName_info;
};
class CanUseHost{
public:
    int node; //1A2B;
    int hostID;
    string hostName;
    vector<int> hostinfo;//CPU MEM服务器总有信息
    vector<int> unSource;//CPU MEM服务器剩余的资源
};

class MigrationInfo{
public:
    int nowHostID;
    int now_node; //1A2B;当前节点
    int targetID; 
    int target_node; //1A2B;目标节点节点
    int vminfo[2];
    string vmName;//迁移id
    float p_level_total;//优先级
    string outInfo;//(虚拟机 ID, 目的服务器 ID)
    float _1_orgin_sim_;
    float _1_target_sim_;
    float _1_total_sim_;
    float _2_orgin_sim_;
    float _2_target_sim_;
    float _2_total_sim_;
};


class Server
{
public:
    void getServerInfo(string &serverName, string &coreNum, string &memorySize, string &serverCost, string &energyCost);
    void buyServer();
    void sim_buyServer(int sourceLack[2], int istwo, int unday);
    bool choseServer(vector<int> &server, vector<int> &vm, int serverId, string vmId, int requestOrder);
    bool sim_choseServer(vector<int> &server, vector<int> &vm, int serverId, string vmId, int requestOrder);

public:
    //服务器信息
    unordered_map<string, vector<int>> m_serverInfos;
    int serverNum_ID = 0;
    int sim_serverNum_ID = 0;

    //记录购买服务器id对应服务器类型
    unordered_map<int, string> m_IDtoServerType;
    unordered_map<int, string> m_sim_IDtoServerType;
    //当前节点服务器id ,剩余资源
    unordered_map<int, vector<int>> m_nowServerSource;
    unordered_map<int, vector<int>> m_sim_nowServerSource;
    //临时记录第i条add信息在什么类型的服务器
    unordered_map<int, string> m_sim_vmIntypeSer;
    // 记录虚拟机运行在那个服务器上
	map<string, vector<int>> vmOnServer;
    // 当前开机服务器
	vector<int> serverRunVms;
    //购买服务器成本 能耗
    long long m_serverCost = 0, m_powerCost = 0, m_totalCost = 0;

    //每一天购买的服务器
    // string buyserverName[5];
    unordered_map<string, int> type_buyServerNum;
    vector<string> test_ser;
    int count = 0;
    //每一天输出信息
    unordered_map<int, string> dayoutInfos;
    //每一天需要的最低服务器限制
    int mr_vmNeedMaxSource[2] = {0, 0};
    
    //记录每一个id服务器挂载的vm台数信息 id - 虚拟机序号
    unordered_map<int, list<string>> serverID_vmInfo;

};

class VM
{
public:
    void getVMInfo(string &vmName, string &vmCoreNum, string &vmMemorySize, string &isTwoNodes);
    
public:
    unordered_map<string, vector<int>> m_vmInfos;
    long long int m_nowVMnum = 0;
};

class Order
{
public:
    void addOrder(string& vmName, string& vmID);
    void delOrder(string& vmID);
public:
    // 当天请求信息
	vector<vector<string>> requestInfos;
	// 当天总请求信息
	vector<vector<vector<string>>> totalRequestInfos;
};

class Compute
{
public:
    Compute();
    ~Compute();
    void initData();
    void process();
    void purchaseServer();
    void requestProcess();
    int createVM(vector<string> &vminfo, int requestOrder, int type);
    int sim_createVM(vector<string> &vminfo, int requestOrder);
    void delVM(vector<string>& delVmInfo);
    void outInfo();
    vector<string> migration(int req);
    map<float, MigrationInfo> migrationStrategy(ZeroCpuHost _zeroCpuSortMem, list<CanUseHost> _ID_canUseSource);

public:
    Server* m_server;
    VM* m_vm;
    Order* m_order;
    vector<string> migration_outInfo;
    
    vector<vector<vector<string>>> totalOutInfos;
};
#endif
// #define TEST
using namespace std;

// 完成服务器信息的读取，并将其存放在m_serverInfos中
void Server::getServerInfo(string &serverName, string &coreNum, string &memorySize,\
string &serverCost, string &energyCost)
{
    string _serverName = "";
    for (int i = 1; i < serverName.size() - 1; i++) {
        _serverName += serverName[i];
    }
    
    int _coreNum = 0, _memorySize = 0, _serverCost = 0, _energyCost = 0;
    _coreNum = stoi(coreNum);
    _memorySize = stoi(memorySize);
    _serverCost = stoi(serverCost);
    _energyCost = stoi(energyCost);
    m_serverInfos[_serverName] = vector<int>{_coreNum / 2, _coreNum / 2, _memorySize / 2,
     _memorySize / 2, _serverCost, _energyCost};
   
    // cout << _serverName << "   "<<_coreNum << "  " << _memorySize << \
    // "  " << _serverCost << "  " << _energyCost << endl;
}



//选择服务器
bool Server::sim_choseServer(vector<int> &server, vector<int> &vm, int serverId, string vmId, int requestOrder)
{
    int vmCores = vm[0], vmMemory = vm[1], vmTwoNodes = vm[2];
    int& serverCoreA = server[0], & serverCoreB = server[1], & serverMemoryA = server[2], & serverMemoryB = server[3];
    if (vmTwoNodes) {
        int needCores = vmCores / 2, needMemory = vmMemory / 2;
        if (serverCoreA >= needCores && serverCoreB >= needCores && serverMemoryA >= needMemory && serverMemoryB >= needMemory) {
            serverCoreA -= needCores;
            serverCoreB -= needCores;
            serverMemoryA -= needMemory;
            serverMemoryB -= needMemory;

            return true;
        }
        else {
            return false;
        }
    }
    else if (serverCoreA >= vmCores && serverMemoryA >= vmMemory) {
        serverCoreA -= vmCores;
        serverMemoryA -= vmMemory;
    
        return true;
    }
    else if (serverCoreB >= vmCores && serverMemoryB >= vmMemory) {
        serverCoreB -= vmCores;
        serverMemoryB -= vmMemory;
        return true;
    }
    return false;
}
//istwo = 1.代表双节点
void Server::sim_buyServer(int sourceLack[2], int istwo, int unday)
{   
    //调参系数

    //
    int purchasenum = 0;
    long long int costPer;
    //满足条件的服务器的性价比
    map<long long int, string> serverCostPer;
    //先遍历所有服务器
    for(auto& l_sever : m_serverInfos){
        string s_name = l_sever.first;
        vector<int> s_info = l_sever.second;
        
        if(s_info[0] >= mr_vmNeedMaxSource[0] && s_info[2] >= mr_vmNeedMaxSource[1]){
            vector<int> serverinfo = m_serverInfos[s_name];//双节点
            int a1 = sourceLack[0] / serverinfo[0] / 2;
            int a2 = sourceLack[1] / serverinfo[2] / 2;
            purchasenum = a1 > a2 ? a1 : a2;
            purchasenum += 1;
            long long int powerCost = serverinfo[5] * purchasenum * unday * 9;
            long long int hardwareCost = serverinfo[4] * (long long int)purchasenum * 20 ;
            costPer = powerCost + hardwareCost;
            serverCostPer[costPer] = s_name;
        }
    } 
    string name;
    map<float, string> serverCostPer_new;//重新计算性价比
    int count = 0;//只看前几个
    for(auto& iter : serverCostPer){
        name = iter.second;
        vector<int> server_info = m_serverInfos[name];
        int totalSource = server_info[0] * 1 + server_info[2] * 2;
        float newValue = (float)totalSource / iter.first * 1.0;
        serverCostPer_new[newValue] = name;
        // if(++count > 5)
            break;
    }

    for(auto& iter : serverCostPer_new){
        name = iter.second;
    }
    vector<int> serverSelectInfo = m_serverInfos[name];//双节点
    int a1 = sourceLack[0] / serverSelectInfo[0] / 2;
    int a2 = sourceLack[1] / serverSelectInfo[2] / 2;
    int buynum = 1;
    buynum += a1 > a2 ? a1 : a2;
    for (int i = 0; i < buynum; i++){
        m_sim_IDtoServerType[sim_serverNum_ID] = name;
        m_sim_nowServerSource[sim_serverNum_ID] = serverSelectInfo;
        sim_serverNum_ID++;
    }
    type_buyServerNum[name] += buynum;
    test_ser.push_back(name);
    serverCostPer.clear();
}

//创建虚拟机requestOrder 请求序号
int Compute::sim_createVM(vector<string> &vminfo, int requestOrder)
{
    string reqVMname = vminfo[1], reqID = vminfo[2];
    vector<int> vmCostSource = m_vm->m_vmInfos[reqVMname];
    int success = -1;

    for (int i = 0; i < m_server->sim_serverNum_ID; i++){
        auto &sever = m_server->m_sim_nowServerSource[i];
        if (m_server->sim_choseServer(sever, vmCostSource, i, reqID, requestOrder)) {

            string name = m_server->m_sim_IDtoServerType[i];
            m_server->m_sim_vmIntypeSer[requestOrder] = name;//记录应该分配到哪种服务器
            success = 1;
            break;
        }
    }  
    return success;
}


//选择服务器 serverId在为运行的服务器编号
bool Server::choseServer(vector<int> &server, vector<int> &vm, int serverId, string vmId, int requestOrder)
{
    int vmCores = vm[0], vmMemory = vm[1], vmTwoNodes = vm[2];
    int& serverCoreA = server[0], & serverCoreB = server[1], & serverMemoryA = server[2], & serverMemoryB = server[3];
    if (vmTwoNodes) {
        int needCores = vmCores / 2, needMemory = vmMemory / 2;
        if (serverCoreA >= needCores && serverCoreB >= needCores && serverMemoryA >= needMemory && serverMemoryB >= needMemory) {
            serverCoreA -= needCores;
            serverCoreB -= needCores;
            serverMemoryA -= needMemory;
            serverMemoryB -= needMemory;

            vmOnServer[vmId] = vector<int>{serverId, needCores, needMemory, 1, 2};
            string s = "(" + to_string(serverId) + ")";
            dayoutInfos[requestOrder] = s;
            return true;
        }
        else {
            return false;
        }
    }
    else if (serverCoreA >= vmCores && serverMemoryA >= vmMemory) {
        serverCoreA -= vmCores;
        serverMemoryA -= vmMemory;
        vmOnServer[vmId] = vector<int>{serverId, vmCores, vmMemory, 1};
        string s = "(" + to_string(serverId) + ", A)";
        dayoutInfos[requestOrder] = s;

        return true;
    }
    else if (serverCoreB >= vmCores && serverMemoryB >= vmMemory) {
        serverCoreB -= vmCores;
        serverMemoryB -= vmMemory;
        vmOnServer[vmId] = vector<int>{serverId, vmCores, vmMemory, 2};
        string s = "(" + to_string(serverId) + ", B)";
        dayoutInfos[requestOrder] = s;

        return true;
    }

    return false;
}

//istwo = 1.代表双节点，真实购买
void Server::buyServer()
{
    
    string namelast;
    // for (int i = 0; i < test_ser.size(); i++){
    //     int num = type_buyServerNum[test_ser[i]];
    //     if(test_ser[i] != namelast){
    //         for (int j = 0; j < num; j++){
    //             m_nowServerSource[serverNum_ID] = m_serverInfos[test_ser[i]];
    //             serverNum_ID++;
    //         }
    //     }
    //     namelast = test_ser[i];
    // }
    for (auto &v : type_buyServerNum) {
        for (int i = 0; i < v.second; i++) {
            m_nowServerSource[serverNum_ID] = m_serverInfos[v.first];
            m_IDtoServerType[serverNum_ID] = v.first;
            serverNum_ID++;
        }
    }
}
//创建虚拟机requestOrder 请求序号
int Compute::createVM(vector<string> &vminfo, int requestOrder, int type)
{
    string reqVMname = vminfo[1], reqID = vminfo[2];
    vector<int> vmCostSource = m_vm->m_vmInfos[reqVMname];
    int success = -1;
    if(type == 0){
        for (int i = 0; i < m_server->serverNum_ID; i++){
            auto &sever = m_server->m_nowServerSource[i];
            if (m_server->choseServer(sever, vmCostSource, i, reqID, requestOrder)) {//i是id
                        // m_server->serverRunVms[i]++;
                success = 1;

                m_server->serverID_vmInfo[i].push_back(reqID);
                break;
            }
        }  
    }
    if(type == 1){//扩充
        int sim_id = m_server->sim_serverNum_ID;
        int _id = m_server->serverNum_ID;
        for (int i = _id - sim_id; i < _id; i++){//只能在这里面扩充
            string shouldname = m_server->m_sim_vmIntypeSer[requestOrder];//应该插入的服务器
            string nowIdservername = m_server->m_IDtoServerType[i];//满足条件才插入
            if(shouldname == nowIdservername){
                auto &sever = m_server->m_nowServerSource[i];
                if (m_server->choseServer(sever, vmCostSource, i, reqID, requestOrder)) {
                            // m_server->serverRunVms[i]++;
                    m_server->serverID_vmInfo[i].push_back(reqID);
                    success = 1;
                    break;
                }
            }         
        }  
    }

    return success;
}

//删除虚拟机
void Compute::delVM(vector<string>& delVmInfo)
{
    string _vmId = delVmInfo[1];
    auto _vmInfo = m_server->vmOnServer[_vmId];
    vector<int> _serverInfo = m_server->vmOnServer[_vmId];
    int _serverId = _serverInfo[0];

    // m_server->serverRunVms[_serverId]--;

    m_server->serverID_vmInfo[_serverId].remove(_vmId);

    if (_serverInfo.size() == 5) {
        vector<int>& server = m_server->m_nowServerSource[_serverId];
        int cores = _vmInfo[1], memory = _vmInfo[2];
        server[0] += cores;
        server[1] += cores;
        server[2] += memory;
        server[3] += memory;
    }
    else {
        vector<int>& server = m_server->m_nowServerSource[_serverId];
        int cores = _vmInfo[1], memory = _vmInfo[2];
        if (_serverInfo[3] == 1) {
            server[0] += cores;
            server[2] += memory;
        }
        else {
            server[1] += cores;
            server[3] += memory;
        }
    }
}

//获取虚拟机部署信息， 0表示单节点部署
void VM::getVMInfo(string &vmName, string &vmCoreNum, string &vmMemorySize, string &isTwoNodes)
{
    string _vmName = "";
    for (int i = 1; i < vmName.size() - 1; i++) {
        _vmName += vmName[i];
    }

    int _vmCoreNum = 0, _vmMemorySize = 0, _isTwoNodes = 0;
    _vmCoreNum = stoi(vmCoreNum);
    _vmMemorySize = stoi(vmMemorySize);
    _isTwoNodes = stoi(isTwoNodes);
    m_vmInfos[_vmName] = vector<int>{_vmCoreNum, _vmMemorySize, _isTwoNodes};
}

void Order::addOrder(string& vmName, string& vmID)
{
    string name = vmName.substr(0, vmName.size() - 1);
    string id = vmID.substr(0, vmID.size() - 1);
    requestInfos.push_back(vector<string>{"1", name, id});
}
void Order::delOrder(string& vmID)
{
    string id = vmID.substr(0, vmID.size() - 1);
    requestInfos.push_back(vector<string>{"0", id});
}

void Compute::initData(){
#ifdef TEST
    const string filepath = "C:\\Users\\zs\\Desktop\\huawei\\training-1.txt";
    freopen(filepath.c_str(), "rb", stdin);
#endif

//服务器信息
    int serverNum;
    string serverinfo[5];
    cin >> serverNum;
    int maxprice = 0;
    string key = "";
    for (int i = 0; i < serverNum; i++){
        cin >> serverinfo[0] >> serverinfo[1] >> serverinfo[2] >> serverinfo[3] >> serverinfo[4];
        m_server->getServerInfo(serverinfo[0], serverinfo[1], serverinfo[2], serverinfo[3], serverinfo[4]);
        
    }

    //虚拟机信息
    int vmNum;
    string vminfo[4];
    cin >> vmNum;
    for (int i = 0; i < vmNum; i++) {
        cin >> vminfo[0] >> vminfo[1] >> vminfo[2] >> vminfo[3];
        m_vm->getVMInfo(vminfo[0], vminfo[1], vminfo[2], vminfo[3]);
    }

//命令信息
    int totalDays, dayInfoNum;//总的请求天数，每一天请求数量
    string isAdd, vmname, vmID;
    cin >> totalDays;
    // 开始处理

    for (int day = 0; day < totalDays; day++) {
        cin >> dayInfoNum;
       
        for (int i = 0; i < dayInfoNum; i++) {
            cin >> isAdd;
            if (isAdd[1] == 'a') {
                cin >> vmname >> vmID;
                m_order->addOrder(vmname, vmID);
            }
            else {
                cin >> vmID;
                m_order->delOrder(vmID);
            }           
        }
        m_order->totalRequestInfos.push_back(m_order->requestInfos);
        m_order->requestInfos.clear();
        

#ifdef TEST
        if (day == 0 || (day + 1) % 100 == 0) {
            printf(" %d day \n", day + 1);
        }
#endif
    }
    fclose(stdin);

}

Compute::Compute(){
    m_server = new Server;
    m_vm = new VM;
    m_order = new Order;
    process();
}


Compute::~Compute(){
    delete m_server;
    delete m_vm;
    delete m_order;
}

void Compute::outInfo(){
    int typeNum = m_server->type_buyServerNum.size();
    cout << "(purchase, " << typeNum << ")" << endl;

    for(auto&v : m_server->type_buyServerNum){
        cout << "(" << v.first << ", " << v.second << ")" << endl;
    }
    int migNum = migration_outInfo.size();
    cout << "(migration, " << migNum << ")"  << endl;
    for(auto&v : migration_outInfo){
        cout << v << endl;
    }
    int size = m_server->dayoutInfos.size();
    for (int i = 0; i < size; i ++){
        if(m_server->dayoutInfos[i] != "%"){
            cout << m_server->dayoutInfos[i] << endl;
        }
    }
    // int liveVMnum = 0;
    // for(auto&v : m_server->serverID_vmInfo){
    //     liveVMnum += v.second.size();
    // }
    // int totalnum = m_server->vmOnServer.size();
    
    m_server->sim_serverNum_ID = 0;
    m_server->dayoutInfos.clear();
    migration_outInfo.clear();
    m_server->type_buyServerNum.clear();
    m_server->test_ser.clear();
    m_server->m_sim_IDtoServerType.clear();
    m_server->m_sim_vmIntypeSer.clear();
    m_server->m_sim_nowServerSource.clear();

}
void Compute::requestProcess()
{
    vector<vector<string>> infos;
    vector<int> vminfo;
    int day = m_order->totalRequestInfos.size(); 
    for (int i = 0; i < day; i++)
    {
        infos = m_order->totalRequestInfos[i];
        int dayNum = infos.size();//每一天的数量
        int completeNum = 0;
        int canMignum = 5 * m_vm->m_nowVMnum / 1000;
        int num = -1;
        int req = 1;
        for (int i = 0; i < canMignum; i++){
            
            
            if(migration_outInfo.size() == num){
                // req++;
                break;
            }
            num = migration_outInfo.size();    
            migration(req);
            m_server->count++;
            if (m_server->count > 12)
            {
                m_server->count = 0;
                break;
            }
            // req = 1;
        }

//先把能加进去的加进去
        for (int j = 0; j < dayNum; j++){
            vector<string> requset_day = infos[j];//每一天的每一条信息遍历
            if(requset_day[0] == "0"){//删除计数
                completeNum++;
                delVM(requset_day);
                m_server->dayoutInfos[j] = "%";
                infos[j][0] = "3";
                m_vm->m_nowVMnum--;
            }
            if(requset_day[0] == "1"){//创建
                vminfo = m_vm->m_vmInfos[requset_day[1]];
                if(createVM(requset_day, j, 0) == 1){//能分配成功,0表示初始插入
                    infos[j][0] = "3";//表示已经处理了
                    completeNum++;                 
                }
                m_vm->m_nowVMnum++;
            }   
        }
        
        //模拟分配一次。确定扩容
        while(completeNum < dayNum){
            //统计单双节点需要总资源,0位代表cpu核数，1位代表内存大小
            int singleNode[2] = {0, 0};
            int twoNode[2] = {0, 0};
            int totalNode_lacksource[2] = {0, 0};
            for (int j = 0; j < dayNum; j++){
                vector<string> requset_day = infos[j];//每一天的每一条信息遍历
                if(requset_day[0]=="3" || requset_day[0]=="4")//==3已经分配，==4模拟分配成功
                    continue;
                if(requset_day[0] == "1"){//创建
                    vminfo = m_vm->m_vmInfos[requset_day[1]];
                    if(sim_createVM(requset_day, j) == -1){//未能分配成功
                        if(vminfo[2] == 0){//单节点统计
                            singleNode[0] += vminfo[0];
                            singleNode[1] += vminfo[1];
                            m_server->mr_vmNeedMaxSource[0] = m_server->mr_vmNeedMaxSource[0] > vminfo[0] ? m_server->mr_vmNeedMaxSource[0] : vminfo[0];
                            m_server->mr_vmNeedMaxSource[1] = m_server->mr_vmNeedMaxSource[1] > vminfo[1] ? m_server->mr_vmNeedMaxSource[1] : vminfo[1];
                        }
                        else{//双节点统计
                            twoNode[0] += vminfo[0] ;
                            twoNode[1] += vminfo[1] ;
                            m_server->mr_vmNeedMaxSource[0] = m_server->mr_vmNeedMaxSource[0] > (vminfo[0] / 2) ? m_server->mr_vmNeedMaxSource[0] : (vminfo[0] / 2);
                            m_server->mr_vmNeedMaxSource[1] = m_server->mr_vmNeedMaxSource[1] > (vminfo[1] / 2) ? m_server->mr_vmNeedMaxSource[1] : (vminfo[1] / 2);
                        }
                    }   
                    else{
                        infos[j][0] = "4";//表示已经处理了
                        completeNum++;
                    }           
                }
            }//遍历结束
            totalNode_lacksource[0] = singleNode[0] + twoNode[0];
            totalNode_lacksource[1] = singleNode[1] + twoNode[1];
            m_server->sim_buyServer(totalNode_lacksource, 0, day - i); 
        }//while
        m_server->buyServer(); //扩容
        //实际分配
        for (int j = 0; j < dayNum; j++){
            vector<string> requset_day = infos[j];//每一天的每一条信息遍历
            if(requset_day[0] == "4"){//创建
                if(createVM(requset_day, j, 1) == -1){//未能分配成功
                    m_server->count++;
                }
            }    
        }
        outInfo();
        m_server->mr_vmNeedMaxSource[0] = 0;
        m_server->mr_vmNeedMaxSource[1] = 0;
    }
}



//迁移模块
//迁移模块
//迁移模块

vector<string> Compute::migration(int req){
    //迁移准备//
    //迁移准备//
    map<int, ZeroCpuHost, greater<int>> _zeroCpuSortMem;//按照0cpu 对应剩余内存最大排序，<剩余值, <host id,a 1 or b 2)>
    map<int, ZeroMemHost, greater<int>> _zeroMemSortCpu;//按照0内存剩余cpu最大排序，<剩余值, <host id,a 1 or b 2)>
    list<CanUseHost> _ID_canUseSource;
    //先找到剩余最大的
    map<int, vector<int>, greater<int>> unserSource;//id a b
    for(auto&iter : m_server->m_nowServerSource){
        if(((iter.second[0]==0)&&(iter.second[2]!=0))||((iter.second[1]==0)&&(iter.second[3]!=0))){
            if(iter.second[0]==0){//0.1cpu
                unserSource[iter.second[2]] = {iter.first, 1, 0, iter.second[2]};
            }
            if(iter.second[1]==0){//0.1cpu
                unserSource[iter.second[3]] = {iter.first, 2, 0, iter.second[3]};
            }
        }

        if(((iter.second[2]==0)&&(iter.second[0]!=0))||((iter.second[3]==0)&&(iter.second[1]!=0))){           
            if(iter.second[2]==0){
                unserSource[iter.second[0]] = {iter.first, 1, iter.second[0], 0};
            }
            if(iter.second[3]==0){
                unserSource[iter.second[1]] = {iter.first, 2, iter.second[1], 0};
            }
        }

        if(iter.second[0]&&iter.second[2]){
            CanUseHost canusehost;
            canusehost.hostID = iter.first;
            canusehost.node = 1;
            canusehost.hostName = m_server->m_IDtoServerType[iter.first];
            canusehost.hostinfo = m_server->m_serverInfos[canusehost.hostName];
            canusehost.unSource = {iter.second[0], iter.second[2]};
            _ID_canUseSource.push_back(canusehost);
        }
        if(iter.second[1]&&iter.second[3]){
            CanUseHost canusehost;
            canusehost.hostID = iter.first;
            canusehost.node = 2;
            canusehost.hostName = m_server->m_IDtoServerType[iter.first];
            canusehost.hostinfo = m_server->m_serverInfos[canusehost.hostName];
            canusehost.unSource = {iter.second[1], iter.second[3]};
            _ID_canUseSource.push_back(canusehost);
        }
    }
    ZeroCpuHost zeroCpuHost;
    for(auto & iter : unserSource){
        int server_ID = iter.second[0];
        string hostname = m_server->m_IDtoServerType[server_ID];
        zeroCpuHost.hostID = server_ID;
        zeroCpuHost.hostName = hostname;
        zeroCpuHost.hostinfo = m_server->m_serverInfos[hostname];
        list<string> vmname = m_server->serverID_vmInfo[server_ID];
        for(auto&v : vmname){             
            vector<int> info = m_server->vmOnServer[v];
            if(info.size()==4){
                if(info[3] == iter.second[1]){
                    zeroCpuHost.vmName_info[v] = {info[1],info[2]};
                }    
            }
        }
        zeroCpuHost.node = iter.second[1];
        zeroCpuHost.unSource = {iter.second[2], iter.second[3]};
        zeroCpuHost.p_level = iter.second[2];
        req--;
        if(req == 0)
            break;
    }

    map<float, MigrationInfo> mig_info;
    mig_info = migrationStrategy(zeroCpuHost, _ID_canUseSource);
    //迁移策略//
    //迁移策略//
    vector<string> res;
    int canMigNum = 5 * m_vm->m_nowVMnum / 1000;
    string s;  
    map<string, int> haveMig;
    for(auto&v : mig_info){
        if(migration_outInfo.size() >= canMigNum){
            break;
        }         
        string vmname = v.second.vmName;
        if(haveMig[vmname] != haveMig.empty()){
            continue;
        }else{
            haveMig[vmname] = 1;
        }
        
        int target_ID = v.second.targetID;
        int target_node = v.second.target_node;
        int orgin_ID = v.second.nowHostID;
        int orgin_node = v.second.now_node;
        int vmInfo[2];
        vmInfo[0] = v.second.vminfo[0];
        vmInfo[1] = v.second.vminfo[1];
        // if(v.first  < 4){
            if(v.second.target_node == 1){
            if((m_server->m_nowServerSource[target_ID][0] >= vmInfo[0]) && (m_server->m_nowServerSource[target_ID][2] >= vmInfo[1])){         
                m_server->vmOnServer[vmname][0] = v.second.targetID;
                m_server->vmOnServer[vmname][3] = 1;
                m_server->m_nowServerSource[target_ID][0] -= vmInfo[0];
                m_server->m_nowServerSource[target_ID][2] -= vmInfo[1];
                if(orgin_node == 1){
                    m_server->m_nowServerSource[orgin_ID][0] += vmInfo[0];
                    m_server->m_nowServerSource[orgin_ID][2] += vmInfo[1];
                }else{
                    m_server->m_nowServerSource[orgin_ID][1] += vmInfo[0];
                    m_server->m_nowServerSource[orgin_ID][3] += vmInfo[1];
                }
                m_server->serverID_vmInfo[orgin_ID].remove(vmname);
                m_server->serverID_vmInfo[target_ID].push_back(vmname);
         
                s = "(" + vmname + ", " + to_string(v.second.targetID) + ", A)"; 
                migration_outInfo.push_back(s);
            }     
            }else if(v.second.target_node == 2){   
            if((m_server->m_nowServerSource[target_ID][1] >= vmInfo[0]) && (m_server->m_nowServerSource[target_ID][3] >= vmInfo[1])){
                m_server->vmOnServer[vmname][0] = v.second.targetID;
                m_server->vmOnServer[vmname][3] = 2;
                m_server->m_nowServerSource[target_ID][1] -= vmInfo[0];
                m_server->m_nowServerSource[target_ID][3] -= vmInfo[1];
                if(orgin_node == 1){
                    m_server->m_nowServerSource[orgin_ID][0] += vmInfo[0];
                    m_server->m_nowServerSource[orgin_ID][2] += vmInfo[1];
                }else{
                    m_server->m_nowServerSource[orgin_ID][1] += vmInfo[0];
                    m_server->m_nowServerSource[orgin_ID][3] += vmInfo[1];
                }
                m_server->serverID_vmInfo[orgin_ID].remove(vmname);
                m_server->serverID_vmInfo[target_ID].push_back(vmname);
                s = "(" + vmname + ", " + to_string(v.second.targetID) + ", B)";
                migration_outInfo.push_back(s);
                } 
            }
        // }
        
        // break;
    }

    return res;
}

map<float, MigrationInfo>Compute::migrationStrategy(ZeroCpuHost _zeroCpuSortMem, list<CanUseHost>_ID_canUseSource){
    map<float, MigrationInfo> res;
    
    
        ZeroCpuHost l_var = _zeroCpuSortMem;//first是优先级，second是信息
        
        for(auto& orgin : l_var.vmName_info){
            
            // float sim_square = l_var.unSource[0] +    
            string vmName = orgin.first;
            vector<int> vmInfo = orgin.second;
            for(auto&v : _ID_canUseSource){
                
                vector<int> un_source = v.unSource;
                if((vmInfo[0]<=un_source[0])&&(vmInfo[1]<=un_source[1])){
                    MigrationInfo mig_info;
                    mig_info.vminfo[0] = vmInfo[0];
                    mig_info.vminfo[1] = vmInfo[1];
                    mig_info.now_node = l_var.node;
                    mig_info.nowHostID = l_var.hostID;
                    mig_info.target_node = v.node;
                    mig_info.targetID = v.hostID;
                    mig_info.vmName = vmName;

                    //这里确定迁移指标前
                    float _orgin_shape = 1;
                    int _orgin_x = l_var.unSource[0] + vmInfo[0];
                    int _orgin_y = l_var.unSource[1] + vmInfo[1];
                    float _uorgin_shape = abs((float)_orgin_x / (float)_orgin_y - 1);//接近正方形
                    if(_orgin_shape < _uorgin_shape)//原始小于迁移后
                        continue;
                    mig_info._1_orgin_sim_ = _orgin_shape;
                    mig_info._2_orgin_sim_ = _uorgin_shape;
                    // float square = exp(-1 * ((float)_orgin_x * (float)_orgin_y));
                    float orgin_score = _orgin_shape - _uorgin_shape ;

                    //迁移后指标
                    float _target_shape = abs((float)v.unSource[0] / (float)v.unSource[1] - 1);
                    _orgin_x = v.unSource[0] - vmInfo[0];
                    _orgin_y = v.unSource[1] - vmInfo[1];
                    float _utarget_shape = abs((float)_orgin_x / (float)_orgin_y - 1);  //迁移后 越小越接近               
                    if(_target_shape < _utarget_shape)//
                        continue;
                    if((_orgin_x == 0) && (_orgin_y != 0))
                        continue;
                    if((_orgin_x != 0) && (_orgin_y == 0))
                        continue;

                    mig_info._1_target_sim_ = _target_shape;
                    mig_info._2_target_sim_ = _utarget_shape;
                    // float square1 = exp(-1 * ((float)_orgin_x * (float)_orgin_y));
                    float target_score = _target_shape - _utarget_shape ;

                    if((_orgin_x == 0) && (_orgin_y == 0)){
                        mig_info.p_level_total = 0;//最高优先级
                    }else{
                        mig_info.p_level_total = 1 / (target_score * 2);
                    }
                    //越xiao代表效果降低越多
                    res[mig_info.p_level_total] = mig_info;
                    v.unSource[0] = v.unSource[0] - vmInfo[0];
                    v.unSource[1] = v.unSource[1] - vmInfo[1];                  
                }
            }
        }
        
    

    // for(auto&iter : _zeroMemSortCpu){
    //     ZeroMemHost l_var = iter.second;//first是优先级，second是信息
    //     for(auto& orgin : l_var.vmName_info){
    //         string vmName = orgin.first;
    //         vector<int> vmInfo = orgin.second;
    //         for(auto&v : _ID_canUseSource){
    //             vector<int> un_source = v.unSource;
    //             if((vmInfo[0]<=un_source[0])&&(vmInfo[1]<=un_source[1])){
    //                 MigrationInfo mig_info;
    //                 mig_info.vminfo[0] = vmInfo[0];
    //                 mig_info.vminfo[1] = vmInfo[1];
    //                 mig_info.now_node = l_var.node;
    //                 mig_info.nowHostID = l_var.hostID;
    //                 mig_info.target_node = v.node;
    //                 mig_info.targetID = v.hostID;
    //                 mig_info.vmName = vmName;

    //                 //这里确定迁移指标前
    //                 float _orgin_shape = 1;
    //                 int _orgin_x = l_var.unSource[0] + vmInfo[0];
    //                 int _orgin_y = l_var.unSource[1] + vmInfo[1];
    //                 float _uorgin_shape = abs((float)_orgin_x / (float)_orgin_y - 1);//接近正方形
    //                 if(_orgin_shape < _uorgin_shape)//原始小于迁移后
    //                     continue;
    //                 mig_info._1_orgin_sim_ = _orgin_shape;
    //                 mig_info._2_orgin_sim_ = _uorgin_shape;

    //                 //迁移后指标
    //                 float _target_shape = abs((float)v.unSource[0] / (float)v.unSource[1] - 1);
    //                 _orgin_x = v.unSource[0] - vmInfo[0];
    //                 _orgin_y = v.unSource[1] - vmInfo[1];

    //                 if((_orgin_x == 0) && (_orgin_y != 0))
    //                     continue;
    //                 if((_orgin_x != 0) && (_orgin_y == 0))
    //                     continue;

    //                 float _utarget_shape = abs((float)_orgin_x / (float)_orgin_y - 1);  //迁移后 越小越接近               
    //                 if(_target_shape < _utarget_shape)//
    //                     continue;
                    
    //                 mig_info._1_target_sim_ = _target_shape;
    //                 mig_info._2_target_sim_ = _utarget_shape;

    //                 mig_info.p_level_total = 1 / (_target_shape + _orgin_shape - _utarget_shape - _uorgin_shape);//越xiao代表效果降低越多
    //                 res[mig_info.p_level_total] = mig_info;
    //                 v.unSource[0] = v.unSource[0] - vmInfo[0];
    //                 v.unSource[1] = v.unSource[1] - vmInfo[1];
    //             }
    //         }
    //     }   
    //     break; 
    // }

    return res;
}


void Compute::process(){

    this->initData();
    requestProcess();
}

int main()
{
    Compute compute;
    return 0;
}
