#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
using namespace std;
struct related_Pages {
    int R_ID;
    string URL;
    string title;
    related_Pages *next;
};
struct Page {
    int P_ID;
    string URL;
    string title;
    time_t visitedAt;
    Page *prev, *next;
    related_Pages *rP_head;
};
struct pages_History {
    Page *head, *tail;
};
struct bookMarkNode {
    Page *page;
    bool favorite;
    int visitCount;
    time_t lastVisited;
    bookMarkNode *prev, *next;
};
struct bookMarkList {
    bookMarkNode *head, *tail;
};
// Converts a date string (DD/MM/YYYY) and time string (HH:MM) into time_t
time_t convertToTime(string dateStr, string timeStr) {
    struct tm t = {0};
    stringstream ss1(dateStr);
    stringstream ss2(timeStr);
    string d, m, y, h, mn;

    getline(ss1, d, '/');
    getline(ss1, m, '/');
    getline(ss1, y, ' ');

    getline(ss2, h, ':');
    getline(ss2, mn, ':');

    t.tm_mday = stoi(d);
    t.tm_mon = stoi(m) - 1;
    t.tm_year = stoi(y) - 1900;
    t.tm_hour = stoi(h);
    t.tm_min = stoi(mn);

    return mktime(&t);
}
// Formats a time_t value into a readable date-time string
string formatDateTime(time_t t) {
    tm *lt = localtime(&t);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M", lt);
    return string(buffer);
}
// Inserts a page node at the end of the browsing history (doubly linked list)
void insertPage(pages_History &hispage,Page *newn){
	newn->next=nullptr;
	newn->prev=nullptr;
	if(hispage.head==nullptr){
		hispage.head=newn;
		hispage.tail=newn;
	}
	else{
        newn->prev=hispage.tail;
		hispage.tail->next=newn;
		hispage.tail=newn;
	}	
}
// Inserts a bookmark node at the end of the bookmark list
void insertbookMark(bookMarkList &bookl, bookMarkNode *newn){
	newn->next=nullptr;
	newn->prev=nullptr;
	if(bookl.head==nullptr){
		bookl.head=newn;
		bookl.tail=newn;
	}
	else{
		newn->prev=bookl.tail;
		bookl.tail->next=newn;
		bookl.tail=newn;
	}	
}
// Inserts a related page into a page’s related pages list (singly linked list)
void insertRelated(Page *page, related_Pages *rel){
    rel->next = page->rP_head;
    page->rP_head = rel;
}
// Finds and returns the bookmark node associated with a given page ID
bookMarkNode* findBookmarkForPage(bookMarkList* bList, int pageID) {
    if (!bList) return nullptr;
    bookMarkNode* cur = bList->head;
    while (cur != nullptr) {
        if (cur->page != nullptr) {
            if (cur->page->P_ID == pageID)
			    return cur;
        }
        cur = cur->next;
    }
    return nullptr;
}
// Parses the input text file and populates browsing history and bookmark lists
void parseFile(pages_History &history, bookMarkList &bList, const string &filename){
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Cannot open file!\n";
        return;
    }
    string line;
    Page *currentPage = nullptr;
    while (getline(file, line))
    {
        if (line.size() == 0) continue;
        if (line[0] == '-')
        {
            stringstream ss(line.substr(1));
            string idStr, url, title, date, timepart;
            getline(ss, idStr, ',');
            getline(ss, url, ',');
            getline(ss, title, ',');
            ss >> date;
            ss >> timepart;
            while (!idStr.empty() && idStr[0] == ' ')
			    idStr = idStr.substr(1);
            while (!url.empty()   && url[0]   == ' ') 
			    url   = url.substr(1);
            while (!title.empty() && title[0] == ' ') 
			    title = title.substr(1);
            Page *p = new Page;
            p->P_ID = stoi(idStr);
            p->URL = url;
            p->title = title;
            p->visitedAt = convertToTime(date, timepart);
            p->rP_head = nullptr;
            p->prev = p->next = nullptr;
            insertPage(history, p);
            currentPage = p;
        }
        else if (line[0] == '*')
        {
            if (currentPage == nullptr)
                continue;
            stringstream ss(line.substr(1));
            string idStr, url, title;
            getline(ss, idStr, ',');
            getline(ss, url, ',');
            getline(ss, title, ',');
            while (!idStr.empty() && idStr[0] == ' ') idStr = idStr.substr(1);
            while (!url.empty()   && url[0]   == ' ') url   = url.substr(1);
            while (!title.empty() && title[0] == ' ') title = title.substr(1);
            related_Pages *rp = new related_Pages;
            rp->R_ID = stoi(idStr);
            rp->URL = url;
            rp->title = title;
            rp->next = nullptr;
            insertRelated(currentPage, rp);
        }
            else if (line[0] == '#')
        {
            stringstream ss(line.substr(1));
            string favStr, visitStr, rest;
            getline(ss, favStr, ',');
            getline(ss, visitStr, ',');
            getline(ss, rest);
            while (!favStr.empty() && favStr[0] == ' ') favStr = favStr.substr(1);
            while (!visitStr.empty() && visitStr[0] == ' ') visitStr = visitStr.substr(1);
            while (!rest.empty() && rest[0] == ' ') rest = rest.substr(1);
            string dateStr, timeStr;
            stringstream ssdt(rest);
            ssdt >> dateStr >> timeStr;
            bookMarkNode *bm = new bookMarkNode;
            bm->page = currentPage; 
            bm->favorite = (favStr == "yes");
            bm->visitCount = stoi(visitStr);
            bm->lastVisited = convertToTime(dateStr, timeStr);
            bm->prev = bm->next = nullptr;
            insertbookMark(bList, bm);
        }

    }

    file.close();
}
// Opens a new page, inserts it into history, and updates bookmark visit data if applicable
void openNewPage(pages_History &history,int id,string LINK,string t,related_Pages* relatedHead,bookMarkList &bList){
	Page *opage=new Page;
	opage->P_ID=id;
	opage->URL=LINK;
	opage->title=t;
	opage->rP_head=relatedHead;
	opage->next=opage->prev=nullptr;
	opage->visitedAt=time(nullptr);
	bookMarkNode* bm = bList.head;
	if(history.head==nullptr){
	   history.head=history.tail=opage;
    }
	else{
		insertPage(history,opage);
	}	  
    while (bm != nullptr) {
        if (bm->page->P_ID == id) {
            bm->visitCount++;
            bm->lastVisited = opage->visitedAt;
            return;
        }
        bm = bm->next;
    }
}
struct RPStack {
    related_Pages* node;
    RPStack* next;
};
// Pushes a related page node onto a stack
void push(RPStack* &top, related_Pages* r) {
    RPStack* n = new RPStack;
    n->node = r;
    n->next = top;
    top = n;
}
// Pops a related page node from the stack
related_Pages* pop(RPStack* &top) {
    if(top == nullptr)
        return nullptr;
    RPStack* temp = top;
    related_Pages* r = temp->node;
    top = top->next;
    delete temp;
    return r;
}
// Returns the page that has the highest number of related pages
Page* getMostRelatedPage(pages_History &history) {
    Page* cur = history.head;
    Page* mostRelated = nullptr;
    int maxCount = 0;
    while (cur != nullptr) {
        RPStack* top = nullptr;
        related_Pages* r = cur->rP_head;
        while (r != nullptr) {
            push(top, r);
            r = r->next;
        }
        int count = 0;
        while (pop(top) != nullptr) {
            count++;
        }
        if (count > maxCount) {
            maxCount = count;
            mostRelated = cur;
        }
        cur = cur->next;
    }
    return mostRelated;
}
// Removes a page node from the browsing history list
void HelperHistory(pages_History &history, Page* node) {
    if (node == nullptr) 
	    return;
    if (history.head == history.tail && history.head == node) {
        delete node;
        history.head = history.tail = nullptr;
        return;
    }
    if (node == history.head) {
        history.head = node->next;
        history.head->prev = nullptr;
        delete node;
        return;
    }
    if (node == history.tail) {
        history.tail = node->prev;
        history.tail->next = nullptr;
        delete node;
        return;
    }
    node->prev->next = node->next;
    node->next->prev = node->prev;
    delete node;
}
// Removes a bookmark node safely from the bookmark list
void HelperMark(bookMarkList &bList, bookMarkNode* node) {
    if (node == nullptr) return;
    if (bList.head == bList.tail && bList.head == node) {
        delete node;
        bList.head = bList.tail = nullptr;
        return;
    }
    if (node == bList.head) {
        bList.head = node->next;
        bList.head->prev = nullptr;
        delete node;
        return;
    }
    if (node == bList.tail) {
        bList.tail = node->prev;
        bList.tail->next = nullptr;
        delete node;
        return;
    }
    node->prev->next = node->next;
    node->next->prev = node->prev;
    delete node;
}
// Deletes a page from history along with its related pages and associated bookmarks
void deletePage(pages_History &history, bookMarkList &bList, int ID) {
    Page* cur = history.head;
    while (cur != nullptr) {
        if (cur->P_ID == ID) {
            Page* toDelete = cur;
            related_Pages* r = toDelete->rP_head;
            while (r != nullptr) {
                related_Pages* nx = r->next;
                delete r;
                r = nx;
            }
            bookMarkNode* bm = bList.head;
            while (bm != nullptr) {
                bookMarkNode* nextBM = bm->next;
                if (bm->page == toDelete) {
                	
                   HelperMark(bList,bm);
                }
                bm = nextBM;
            }
            HelperHistory(history, toDelete);
            return;
        }
        cur = cur->next;
    }
}
// Removes all pages visited before a specified cutoff date
void removeOldPages(pages_History &history, bookMarkList &bList, time_t cutoff) {
    Page* cur = history.head;
    while (cur != nullptr) {
        Page* nextNode = cur->next;
        if (cur->visitedAt < cutoff) {
            deletePage(history, bList, cur->P_ID);
        }
        cur = nextNode;
    }
}
// Returns a list of the N most recently visited bookmarks
bookMarkNode* getRecentBookmarks(bookMarkList &bList, int N) {
    bookMarkNode* resultHead = nullptr;
    bookMarkNode* resultTail = nullptr;
    for (int i = 0; i < N; i++) {
        bookMarkNode* maxNode = nullptr;
        bookMarkNode* bm = bList.head;
        while (bm != nullptr) {
            if (bm->lastVisited != -1 && 
               (maxNode == nullptr || bm->lastVisited > maxNode->lastVisited)) {
                maxNode = bm;
            }
            bm = bm->next;
        }
        if (maxNode == nullptr) break;
        bookMarkNode* newNode = new bookMarkNode;
        *newNode = *maxNode;
        newNode->next = nullptr;
        newNode->prev = resultTail;
        if (resultHead == nullptr)
            resultHead = newNode;
        else
            resultTail->next = newNode;
        resultTail = newNode;
        maxNode->lastVisited = -1;
    }
    return resultHead;
}
// Prints the entire browsing history along with related pages
void printHistory(pages_History &history) {
    cout << "---- History ----\n";
    Page* p = history.head;
    while (p!=nullptr) {
        cout << p->P_ID << " | " << p->URL << " | " << p->title << " | visitedAt=" << p->visitedAt << "\n";
        related_Pages* r = p->rP_head;
        while (r) {
            cout<< "   * " <<r->R_ID<<" | " << r->URL << " | " << r->title << "\n";
            r = r->next;
        }
        p = p->next;
    }
}
// Prints all bookmarks with visit count and last visited date
void printBookmarks(bookMarkList &bList) {
    cout << "---- Bookmarks ----\n";
    bookMarkNode* bm=bList.head;
    while (bm) {
        cout<<(bm->favorite?"[fav]" : "") <<bm->page->title<<" | visits=" << bm->visitCount <<" | last="<< formatDateTime(bm->lastVisited)<< "\n";
        bm = bm->next;
    }
}
// Searches browsing history for pages whose URL contains a given substring
void searchPagesBySubstring(pages_History* history, string query) {
    if (!history || !history->head) {
        return;
    }
    Page* cur= history->head; 
    bool found=false;
    cout << "--- Search Results for '" << query << "' ---" << endl;
    while (cur!= nullptr) {
        if (cur->URL.find(query) != string::npos) {
            cout << "ID: " << cur->P_ID 
                 << " | Title: " << cur->title 
                 << " | URL: " << cur->URL << endl;
            found=true;
        }
        cur=cur->next;
    }
    if (found==false) {
        cout << "No pages found matching the query." << endl;
    }
}
// Removes the least visited bookmark from the bookmark list
void removeLeastVisitedBookmark(bookMarkList* list) {
    if (list->head==nullptr) return;
    bookMarkNode *current =list->head;
    bookMarkNode *todelete =list->head;
    int minVisits=list->head->visitCount;
    while (current!=nullptr) {
        if (current->visitCount < minVisits) {
            minVisits=current->visitCount;
            todelete=current;
        }
        current=current->next;
    }
    cout << "Removing bookmark: "<< todelete->page->title << " (Visits: " << minVisits << ")" << endl;
    if (todelete->prev!=nullptr) {
        todelete->prev->next = todelete->next;
    } else {
        list->head=todelete->next;
    }
    if (todelete->next) {
        todelete->next->prev = todelete->prev;
    } else {
        list->tail = todelete->prev;
    }
    delete todelete;
}
struct savedID {
    int id;
    savedID* next;
};

bool alreadySaved(int id, savedID* head) {
    savedID* temp = head;
    while (temp != nullptr) {
        if (temp->id == id) return true;
        temp = temp->next;
    }
    return false;
}

void addSavedID(int id, savedID* &head) {
    savedID* newID = new savedID;
    newID->id = id;
    newID->next = head;
    head = newID;
}
// Saves browsing history, related pages, and bookmarks back to a file without duplication
void saveToFile(pages_History* hList, bookMarkList* bList, string filename) {
    ofstream out(filename);
    if (!out.is_open()) {
        cout << "Error opening file.\n";
        return;
    }
    savedID* savedPages = nullptr;
    savedID* savedRelated = nullptr;
    Page* p = hList->head;
    while (p != nullptr) {
        if (!alreadySaved(p->P_ID, savedPages)) {
            out << "-" << p->P_ID << ", " << p->URL << ", " << p->title 
                << ", " << formatDateTime(p->visitedAt) << "\n";
            addSavedID(p->P_ID, savedPages);
        }
        related_Pages* r = p->rP_head;
        while (r != nullptr) {
            if (!alreadySaved(r->R_ID, savedRelated)) {
                out << "*" << r->R_ID << ", " << r->URL << ", " << r->title << "\n";
                addSavedID(r->R_ID, savedRelated);
            }
            r = r->next;
        }
        bookMarkNode* bm = bList->head;
        while (bm != nullptr) {
            if (bm->page != nullptr && bm->page->P_ID == p->P_ID) {
                out << "#" << (bm->favorite ? "yes" : "no") << ", " 
                    << bm->visitCount << ", " << formatDateTime(bm->lastVisited) << "\n";
            }
            bm = bm->next;
        }
        p = p->next;
    }
    out.close();
    cout << "Data saved.\n";
}
// Main menu-driven function that allows user interaction with the system
int main() {
    pages_History myHistory = {nullptr, nullptr};
    bookMarkList myBookmarks = {nullptr, nullptr};
    string filename = "datafile.txt";
    parseFile(myHistory, myBookmarks, filename);
    string outName= "dataout.txt";
    saveToFile(&myHistory,&myBookmarks,outName);
    int choice = -1;
    while (choice != 0) {
        cout<< "\n=========================================" << endl;
        cout<< "1. Create/Open Page" << endl;
        cout<< "2. Remove History Older Than Date" << endl;
        cout<< "3. Find Most Related Page" << endl;
        cout<< "4. Delete Page" << endl;
        cout<< "5. Show Recent Bookmarks" << endl;
        cout<< "6. Search History" << endl;
        cout<< "7. Remove Least Visited Bookmark" << endl;
        cout<< "8. View History pages" << endl;
        cout<< "9. View BookMarks" <<endl;
        cout<<"10. Save changes" <<endl;
        cout<<"11. Delete a bookMark"<<endl;
        cout <<"0. Exit" << endl;
        cout << "Enter choice: ";
        if (!(cin >> choice)) {
		cin.clear(); 
		cin.ignore(1000, '\n');
		cin.ignore(1000,' ');
		continue;
		}
        switch (choice) {
            case 1: {
                int pid;
                string url, title;
                cout << "Enter Page ID: "; cin >> pid;
                cout << "Enter URL: "; cin >> url;
                cout << "Enter Title: "; cin.ignore(); getline(cin, title);
                related_Pages* rHead = nullptr;
                char addRel;
                cout << "Add related page? (y/n): "; cin >> addRel;
                while(addRel == 'y' || addRel == 'Y') {
                    int rid; string rurl, rtitle;
                    cout << "  Related ID: "; cin >> rid;
                    cout << "  Related URL: "; cin >> rurl;
                    cout << "  Related Title: "; cin.ignore(); getline(cin, rtitle);
                    related_Pages* newRel = new related_Pages;
                    newRel->R_ID = rid; newRel->URL = rurl; newRel->title = rtitle;
                    newRel->next = rHead;
                    rHead = newRel;
                    cout << "Add another related? (y/n): "; cin >> addRel;
                }
                openNewPage(myHistory,pid, url, title, rHead,myBookmarks);
                cout << "Page added." << endl;
                break;
            }
            case 2: {
                string d, t;
                cout << "Enter Date (DD/MM/YYYY): "; cin >> d;
                cout << "Enter Time (HH:MM): "; cin >> t;
                time_t cutoff = convertToTime(d, t);
                removeOldPages(myHistory, myBookmarks, cutoff);
                cout << "Old pages removed." << endl;
                break;
            }
            case 3: {
                Page* p = getMostRelatedPage(myHistory);
                if(p) cout << "Most Related: " << p->title << endl;
                else cout << "No pages found." << endl;
                break;
            }
            case 4: {
                int id;
                cout << "Enter Page ID to delete: "; cin >> id;
                deletePage(myHistory, myBookmarks, id);
                cout << "Delete operation complete." << endl;
                break;
            }
            case 5: {
                int n;
                cout << "How many bookmarks? "; cin >> n;
                bookMarkNode* list = getRecentBookmarks(myBookmarks, n);
                cout << "--- Recent " << n << " Bookmarks ---" << endl;
                while(list) {
                    cout << list->page->title << " (" << formatDateTime(list->lastVisited)<< ")" << endl;
                    list = list->next; 
                }
                break;
            }
            case 6: {
                string q;
                cout << "Enter substring: "; cin >> q;
                searchPagesBySubstring(&myHistory, q);
                break;
            }
            case 7: {
                removeLeastVisitedBookmark(&myBookmarks);
                break;
            }
            case 8: {
                printHistory(myHistory);
                break;
            }
            case 9: {
            	printBookmarks(myBookmarks);
				break;
			}
			case 10:{
				saveToFile(&myHistory,&myBookmarks,outName);
				break;
			}
            case 0:
                cout << "Exiting..." << endl;
            default:
                cout << "Invalid choice." << endl;
        }
    }
    return 0;
}
