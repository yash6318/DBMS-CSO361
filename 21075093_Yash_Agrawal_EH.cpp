#include <bits/stdc++.h>
#include<ext/pb_ds/assoc_container.hpp>
#include<ext/pb_ds/tree_policy.hpp>
#define endl '\n'
typedef long long ll;
using namespace std;
#define pb push_back
#define take(a,n) for(int j=0;j<n;j++) cin>>a[j];
#define give(a,n) for(int j=0;j<n;j++) cout<<a[j]<<' ';
using namespace __gnu_pbds;
// const int M = 998244353;
const int M = 1e9+7;
const int N=2e5+10;

typedef tree<int, null_type, less<int>, rb_tree_tag, tree_order_statistics_node_update> pbds; 
// find_by_order, order_of_key

ll binexp(ll a,ll b){
    ll ans=1;
    while(b){
        if(b&1){
            ans= (ans*a)%M;
        }
        a= (a*a)%M;
        b>>=1;
    }
    return ans;
}

class Bucket{
    ll size;
    ll depth;
public:
    vector<ll> values;

    
    Bucket(ll depth,ll size) {
        this->size = size;
        this->depth = depth;
    }

    ~Bucket() {
        this->values.clear();
    }

    ll get_bucket_depth(){
        return this->depth;
    }


    int insert(ll value){
        int ok = 1;
        int index = find_index(value);

        // if already present, then update
        if (index >= 0) {
            this->update_value(index,value);
            ok = 1;
        }
        else if (size > this->values.size() ){
            this->values.push_back(value);
            ok = 1;
        }
        else ok = 0;

        return ok;
    }

    int update_value(ll key, ll value){
        int ctr=0;
        for (ll &p : this->values){
            if (ctr == key){
                p = value;
                return 1;
            }
            ctr++;
        }

        return 0;
    }

    int merge_bucket(Bucket *b){
        int ok = 1;
        for (ll p : b->values){
            if (!this->insert(p))
                ok = 0;
        }
        // reduce the depth by 1
        this->depth--;

        return ok;
    }


    ll find_index(ll key){
        ll ind = 0;
        for (ll &p : this->values){
            if (p == key) return ind;
            ind++;
        }

        return -1LL; // not found
    }

    int remove(ll key){
        int ok = 1;
        if (find_index(key) >= 0){
            auto it = find(this->values.begin(),this->values.end(),key);
            this->values.erase(it);
            ok = 1;
        }
        else ok = 0;
        
        return ok;
    }
    void print_bucket(){
        cout << " ";
        for (auto &p : this->values){
            cout << "[" << p << "] ";
        }
        cout << endl;
    }
};

class Directory{
    ll bucket_size;
    ll global_depth;
    vector<Bucket *> buckets; 

public:
    Directory(){
        this->global_depth = 1;
        this->bucket_size = 2;
    }
    Directory(ll depth, ll bucket_size){
        this->bucket_size = bucket_size;
        this->global_depth = depth;

        for (ll i = 0; i < (1LL << this->global_depth); i++){
            this->buckets.push_back(new Bucket(depth, bucket_size));
        }
    }
    ~Directory(){
        this->buckets.clear();
    }

    void print_directory(){
        ll bucket_no = 0;
        cout << "Directory: \t Buckets:\n";
        for (Bucket *b : this->buckets){
            cout << bucket_no++ << "\t\t";
            b->print_bucket();
        }
    }


    void double_directory(){
        vector<Bucket *> now = this->buckets;

        for(auto b:now){
            this->buckets.push_back(b);
        }        

        // increase global depth by 1
        this->global_depth++;

    }

    ll calculate_hash(ll n) { 
        // if global depth is x , it returns x LSBs
        return n &((1LL<<global_depth) - 1); 
    }

    ll get_index(ll value){
        // gets index of the bucket in which given value should be there
        return (this->calculate_hash(value) % (1LL << this->global_depth));
    }

    void insert(ll value){
        ll ind = this->get_index(value);

        int ok = this->buckets[ind]->insert(value);

        if (!ok){
            if (this->buckets[ind]->get_bucket_depth() >= this->global_depth){
                this->double_directory();
            }
            else{
                this->split(ind);
            }

            this->insert(value);
        }

    }
    int remove(ll key){
        int ok = 0;
        ll ind = this->get_index(key);
        if (this->buckets[ind]->remove(key)) ok = 1;
        
        this->merge_buckets();
        int can_reduce = this->reduce();

        while(can_reduce){
            this->merge_buckets();
            can_reduce = this->reduce();
        }

        return ok;
    }

    void merge_buckets(){

        ll half = 1LL <<(this->global_depth-1), i=0;

        while(i < half){
            ll size1 = this->buckets[i]->values.size();
            ll size2 = this->buckets[i + half]->values.size();

            if (this->buckets[i] != this->buckets[i + half] &&
                (size1 + size2 <= this->bucket_size)
                ) {
                    this->buckets[i]->merge_bucket(this->buckets[i + half]);

                    delete this->buckets[i + half];

                    this->buckets[i + half] = this->buckets[i];
                }
            i++;
        }

    }
    void split(ll b_ind){
        ll local_depth = this->buckets[b_ind]->get_bucket_depth();

        // the first bucket of that kind
        ll b_first = b_ind % (1LL << local_depth);
        Bucket *b = this->buckets[b_first];

        ll step = 1LL << local_depth;

        Bucket *new1 = new Bucket(local_depth + 1, this->bucket_size);
        Bucket *new2 = new Bucket(local_depth + 1, this->bucket_size);

        ll index = b_first;

        for(ll i = 0; index < (1LL << this->global_depth); i++){
            if(i % 2 == 0) this->buckets[index] = new1;
            else this->buckets[index] = new2;
            index += step;
        }

        for(ll p : b->values) this->insert(p);

        delete b;

    }
    int reduce(){
        int flag = 1;
        for (Bucket *b : this->buckets){
            if(this->global_depth <= b->get_bucket_depth() ){
                flag = 0;
                break;
            }
        }
        if (flag){

            ll half = (1LL << (this->global_depth-1));

            while(half--){
                this->buckets.pop_back();
            }

            this->global_depth--;
        }

        return flag;

    }
};

void Terminal_Wizard(){
    cout<< "\n\n------Terminal Wizard------\n\n";

    cout << "CommandSyntax \tAction\n"
         << "---------------------------\n"
         << "p         \t Print the directory structure.\n"
         << "i <value> \t Insert a record with record number 'value'.\n"
         << "d <value> \t Delete a record with record number 'value'.\n"
         << "h         \t Print the wizard again to show available commands.\n"
         << "q         \t Quit the app.\n";
        
    cout<<"\n\nTip: Just write the command with its arguments in the prompt.\n\n";

}

signed main(){
    cout << "\n----------------Extendible Hashing by 21075093 ---------------\n";

    cout<<"\n\nNote: In this implementation of extendible hashing, we are assuming "
            "only the record number R and visualizing the hashing wrt it as given in the "
            "class presentation\n\n";
    
    ll directory_depth, bucket_size;

    cout << "Enter Initial Directory global depth (d): ";
    cin >> directory_depth;
    cout << "Enter Bucket Size: ";
    cin >> bucket_size;

    Directory d(directory_depth, bucket_size);

    Terminal_Wizard();

    while (1) {
        char choice;
        ll value;

        cout << "Enter Command: > ";
        cin >> choice;

        switch (choice) {
            
        case 'i':
            cin >> value;
            d.insert(value);
            break;

        case 'd':
            cin >> value;
            d.remove(value);
            break;

        case 'p':
            d.print_directory();
            break;

        case 'q':
            cout<<"Quitting..."<<endl;
            break;

        case 'h':
            Terminal_Wizard();
            break;

        default:
            cout << "\nCommand: "<<choice<<" not found! Try reading list of available commands using command 'h'\n\n";
            break;
        }

        if(choice == 'q') break;       

    }
    cout<<"bye!"<<endl;
}