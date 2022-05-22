//
// Created by shaul on 09/05/2022.
//
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <stack>
#include <queue>

using namespace std;

namespace ariel {
    template<class T=string>
    class OrgChart {
        struct Node {
            T data;
            vector<Node *> sons;
            Node *father;

            Node(T &info) : data(info), father(nullptr) {}
        };

        Node *p_root;
    public:
        OrgChart() {
            p_root = nullptr;
        };

        OrgChart(OrgChart<T> &other) {
            this->p_root = other.p_root;
        }

        OrgChart(OrgChart<T> &&other) noexcept {
            this->p_root = other.p_root;
            other.p_root = nullptr;
        }

        /**
         * Base function to add/change root of organizational tree
         * @param root_data -> receive the data to input in the tree
         * @return this instance so we can chain actions
         */
        OrgChart &add_root(T root_data) {
            //if the org chart is empty
            if (this->p_root == nullptr) {
                this->p_root = new Node(root_data);
            }
                //there is a root already in the orgchart
            else {
                //change the data of the root
                this->p_root->data = root_data;
            }
            return *this;
        }

        OrgChart &add_sub(T parent_data, T son_data) {
            //if no root throw argument
            if (p_root == nullptr) {
                throw invalid_argument("No Chart");
            }
            // search if parent is in organization if so add son else throw error
            bool parent_found = false;
            stack<Node *> traversing_stk;
            Node *temp = p_root;
            traversing_stk.push(temp);
            while (!traversing_stk.empty()) {
                if (temp->data == parent_data) {
                    parent_found = true;
                    //add node to parent's sons
                    Node *new_node = new Node(son_data);
                    temp->sons.push_back(new_node);
                    //point parent of son
                    //check this works
                    (new_node)->father = temp;
                    break;
                }
                //add all temps sons
                for (auto &son: temp->sons) {
                    traversing_stk.push(son);
                }
                temp = traversing_stk.top();
                traversing_stk.pop();
            }
            if (!parent_found) {
                throw invalid_argument("No parent found");
            }
            return *this;
        }

        friend ostream &operator<<(ostream &output, const OrgChart &orgChart) {
            PrintTree(output, orgChart.p_root, "", true);
            return output;
        }

        static ostream &PrintTree(ostream &output, Node *p_node, string move, bool last) {
            output << move + "@= " + p_node->data +"\n";
            move += last ? "   " : "|  ";
            for (size_t i = 0; i < p_node->sons.size(); i++) {
                PrintTree(output, p_node->sons[i], move, i == p_node->sons.size() - 1);
            }
            return output;
        }


        // destructor iterates through all the kids as well
        ~OrgChart() {
            stack<Node *> pre_stack;
            stack<Node *> pre_que;
            pre_stack.push(p_root);
            while (!pre_stack.empty()) {
                Node *temp = pre_stack.top();
                pre_stack.pop();
                if (!temp->sons.empty()) {
                    for (auto i = temp->sons.size() - 1; i > 0; i--) {
                        pre_stack.push(temp->sons[i]);
                    }
                    pre_stack.push(temp->sons[0]);
                }
                pre_que.push(temp);
            }
            while (!pre_que.empty()) {
                Node *temper = pre_que.top();
                pre_que.pop();
                delete (temper);
            }
        }

        //-------------------------------------------------------------------
        // Create a iterator sub_class which will be implemented by inner class
        //-------------------------------------------------------------------
        class iterator {
            // 0 is level order
            // 1 is reverse level order
            // 2 is preorder
        private:
            Node *pointer_to_node;
            queue<Node *> lvl_o_que;
            queue<Node *> rvrs_que;
            stack<Node *> rvrs_stack;
            queue<Node *> pre_que;
            stack<Node *> pre_stack;
            int order_type;
        public:
            /**
             * Constructor for iterator. here we will initialize stacks which we are using to help iterate
             * @param ptr Pointer which is used to instantiate iterator
             * @param order This is my helper enum to tell me which case of traversing are we on
             */
            iterator(Node *ptr = nullptr, int order = 1) :
                    pointer_to_node(ptr), order_type(order) {
                //check if ptr points to something
                if (pointer_to_node != nullptr) {
                    //check if ptr has kids
                    if (!pointer_to_node->sons.empty()) {
                        //check what case are we on
                        if (order_type == 0) {//level order
                            for (auto &son: pointer_to_node->sons) {
                                lvl_o_que.push(son);
                            }
                        } else if (order_type == 1) {//reverse level order
                            //in constructor add everything needed to queue 1
                            //in ++ we will move to queue 2 which will be by reverse level order
                            rvrs_que.push(pointer_to_node);
                            while (!rvrs_que.empty()) {
                                Node *temp = rvrs_que.front();
                                rvrs_que.pop();
                                if (!temp->sons.empty()) {
                                    for (auto i = temp->sons.size() - 1; i > 0; i--) {
                                        rvrs_que.push(temp->sons[i]);
                                    }
                                    rvrs_que.push(temp->sons[0]);
                                }
                                rvrs_stack.push(temp);
                            }
                            pointer_to_node = rvrs_stack.top();
                            rvrs_stack.pop();
                        } else {//preorder
                            pre_stack.push(pointer_to_node);
                            while (!pre_stack.empty()) {
                                Node *temp = pre_stack.top();
                                pre_stack.pop();
                                if (!temp->sons.empty()) {
                                    for (auto i = temp->sons.size() - 1; i > 0; i--) {
                                        pre_stack.push(temp->sons[i]);
                                    }
                                    pre_stack.push(temp->sons[0]);
                                }
                                pre_que.push(temp);
                            }
                            //already prints root so remove root from traversing queue
                            pre_que.pop();
                        }
                    }
                }
            }

            T &operator*() const {
                //return *pointer_to_current_node;
                return pointer_to_node->data;
            }

            T *operator->() const {
                return &(pointer_to_node->data);
            }

            // ++i;
            iterator &operator++() {
                //not sure how to do this operator
                // i think i will have to work with getting the index of the son somehow
                //pointer_to_node = pointer_to_node->sons[0];
                if (!lvl_o_que.empty() || !rvrs_stack.empty() || !pre_que.empty()) {
                    if (order_type == 0) {//level order
                        //move pointer to next
                        pointer_to_node = lvl_o_que.front();
                        lvl_o_que.pop();
                        //add sons of new pointer to stack
                        if (pointer_to_node != nullptr) {
                            if (!pointer_to_node->sons.empty()) {
                                for (auto &son: pointer_to_node->sons) {
                                    lvl_o_que.push(son);
                                }
                            }
                        }
                    } else if (order_type == 1) {//reverse level order
                        pointer_to_node = rvrs_stack.top();
                        rvrs_stack.pop();
                    } else {//preorder
                        pointer_to_node = pre_que.front();
                        pre_que.pop();
                    }
                    return *this;
                }
                pointer_to_node = nullptr;
                return *this;
            }

            bool operator==(const iterator &rhs) const {
                return pointer_to_node == rhs.pointer_to_node;
            }

            bool operator!=(const iterator &rhs) const {
                return pointer_to_node != rhs.pointer_to_node;
            }

        };

        iterator begin_level_order() {
            return (iterator{p_root, 0});
        }

        iterator end_level_order() {
            return (iterator{nullptr, 0});
        }

        iterator begin_reverse_order() {
            return (iterator{p_root, 1});
        }

        iterator reverse_order() {
            //not sure about this because we need to stop when we reach root
            return (iterator{nullptr, 1});
        }

        iterator begin_preorder() {
            return (iterator{p_root, 2});
        }

        iterator end_preorder() {
            return (iterator{nullptr, 2});
        }

        //for the for each loop. Should work like level order
        iterator begin() {
            return (iterator{p_root, 0});
        }

        iterator end() {
            return (iterator{nullptr, 0});
        }
    };

}
/*
 * //
// Created by shaul on 19/05/2022.
//
#include <iostream>
#include "OrgChart.hpp"
#include <string>
using namespace std;
using namespace ariel;

int main(){
    //creating a simple procedure where the user can decide what size and what pattern would he like his mat.
    int flag=1;
    OrgChart<string> chart1;
    while(flag) {//when the user decides we exit the loop
        int num1, num2;
        string node1, node2;
        cout << "For Adding A new root enter 1" << endl;
        cout << "For Adding a new child anywhere in the orgchart enter 2" << endl;
        cout << "To print your organizational chart enter 3" << endl;
        int option=1;
        cin >> option;
        if (option == 1) {
            cout<<"Please enter the name of the new root"<<endl;
            try {//making sure to wrap in try and catch in case the user entered illegal arguments
                cin >> node1;
                chart1.add_root(node1);
            }
            catch (exception &ex) {
                cout << "   caught exception : " << ex.what() << endl;
            }
        }
        else if (option == 2) {
            cout<<"Please enter the name of the existing father node"<<endl;
            try {//making sure to wrap in try and catch in case the user entered illegal arguments
                cin >> node1;
                cout<<"Please enter the name of the new son node"<<endl;
                cin >> node2;
                chart1.add_sub(node1,node2);
            }
            catch (exception &ex) {
                cout << "   caught exception : " << ex.what() << endl;
            }
        }
        else { //option 3 print org chart
            cout<<"\nThis is your organizational chart!!\n"<<endl;
            cout<< chart1 <<endl;
        }

        cout << "\nIf you wish to add additional data to your orgchart enter 1, if you wish to exit enter 0" << endl;
        cin>>flag;
        if (!flag){//if he is done we break the loop and exit the program.
            break;
        }
    }
    return 1;

}

 main: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

 */
