#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <conio.h>

#define MAX_NAME 50
#define MAX_PROB 100
#define HEAP_CAP 1000

void get_console_size(int *cols, int *rows) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        *cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        *rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    } else {
        *cols = 120;
        *rows = 30;
    }
}

void gotoxy(int x,int y) {
    int cols, rows;
    get_console_size(&cols, &rows);
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= cols) x = cols - 1;
    if (y >= rows) y = rows - 1;
    COORD CRD = {
        (SHORT)x, (SHORT)y
    };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), CRD);
}


void setcolor (int ForgC)
{
    WORD wColor;
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if(GetConsoleScreenBufferInfo(hStdOut, &csbi))
    {
        wColor = (csbi.wAttributes & 0xF0) + (ForgC & 0x0F);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), wColor);
    }
}

void draw_box(int left, int top, int width, int height) {
    int cols, rows;
    get_console_size(&cols, &rows);

    if (width < 2) width = 2;
    if (height < 2) height = 2;

    if (left < 0) left = 0;
    if (top < 0) top = 0;
    if (left + width >= cols) left = max(0, cols - width - 1);
    if (top + height >= rows) top = max(0, rows - height - 1);

    int right = left + width;
    int bottom = top + height;
    setcolor(14);
    gotoxy(left, top);
    printf("%c", 201);
    gotoxy(right, top);
    printf("%c", 187);
    gotoxy(left, bottom);
    printf("%c", 200);
    gotoxy(right, bottom);
    printf("%c", 188);

    for (int x = left+1; x < right; x++) {
        gotoxy(x, top);
        printf("%c",205);
        gotoxy(x, bottom);
        printf("%c",205);
    }
    for (int y = top+1; y < bottom; y++) {
        gotoxy(left, y);
        printf("%c",186);
        gotoxy(right,y);
        printf("%c",186);
    }
    setcolor(7);
}

void draw_centered_box(int width, int height, int *out_left, int *out_top) {
    int cols, rows;
    get_console_size(&cols, &rows);
    if (width > cols - 4) width = cols - 4;
    if (height > rows - 4) height = rows - 4;
    int left = (cols - width) / 2;
    int top  = (rows - height) / 2;
    draw_box(left, top, width, height);
    if (out_left) *out_left = left;
    if (out_top) *out_top = top;
}

void print_in_box_center(int left, int top, int width, int height, int rel_row, const char *text, int color) {
    int maxlen = width - 2;
    char buf[2048];
    if ((int)strlen(text) > maxlen) {
        strncpy(buf, text, maxlen - 3);
        buf[maxlen - 3] = '\0';
        strcat(buf, "...");
    }
    else strcpy(buf, text);

    int cols, rows;
    get_console_size(&cols, &rows);
    int box_left = left + 1;
    int box_top = top + 1;
    int box_row = box_top + rel_row;
    int x = box_left + ( (maxlen - (int)strlen(buf)) / 2 );
    setcolor(color);
    gotoxy(x, box_row);
    printf("%s", buf);
    setcolor(7);
}

typedef struct NormalPatient {
    int id;
    char name[MAX_NAME];
    char problem[MAX_PROB];
    struct NormalPatient *next;
} NormalPatient;

NormalPatient *qFront = NULL, *qRear = NULL;
int nextNormalId = 1;

void enqueue_normal(const char *name, const char *problem) {
    NormalPatient *p = (NormalPatient*) malloc(sizeof(NormalPatient));
    p->id = nextNormalId++;
    strncpy(p->name, name, MAX_NAME-1);
    p->name[MAX_NAME-1] = '\0';
    strncpy(p->problem, problem, MAX_PROB-1);
    p->problem[MAX_PROB-1] = '\0';
    p->next = NULL;
    if (!qFront) qFront = qRear = p;
    else
    {
        qRear->next = p;
        qRear = p;
    }
}

NormalPatient* dequeue_normal() {
    if (!qFront) return NULL;
    NormalPatient *ret = qFront;
    qFront = qFront->next;
    if (!qFront) qRear = NULL;
    ret->next = NULL;
    return ret;
}

typedef struct EmergencyPatient {
    int id;
    int priority;
    char name[MAX_NAME];
    char problem[MAX_PROB];
} EmergencyPatient;

EmergencyPatient heapArr[HEAP_CAP];
int heapSize = 0;
int nextEmergencyId = 1;

void swapEP(EmergencyPatient *a, EmergencyPatient *b) {
    EmergencyPatient tmp = *a;
    *a = *b;
    *b = tmp;
}

void heapify_up(int idx) {
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (heapArr[parent].priority >= heapArr[idx].priority) break;
        swapEP(&heapArr[parent], &heapArr[idx]);
        idx = parent;
    }
}

void heapify_down(int idx) {
    while (1) {
        int left = 2*idx + 1, right = 2*idx + 2, largest = idx;
        if (left < heapSize && heapArr[left].priority > heapArr[largest].priority) largest = left;
        if (right < heapSize && heapArr[right].priority > heapArr[largest].priority) largest = right;
        if (largest == idx) break;
        swapEP(&heapArr[largest], &heapArr[idx]);
        idx = largest;
    }
}

void push_emergency(const char *name, const char *problem, int priority) {
    if (heapSize >= HEAP_CAP) {
        int l,w;
        draw_centered_box(60,6,&l,&w);
        print_in_box_center(l,w,60,6,2,"Emergency list full!",12);
        getch();
        return;
    }
    EmergencyPatient e;
    e.id = nextEmergencyId++;
    e.priority = priority;
    strncpy(e.name, name, MAX_NAME-1);
    e.name[MAX_NAME-1] = '\0';
    strncpy(e.problem, problem, MAX_PROB-1);
    e.problem[MAX_PROB-1] = '\0';
    heapArr[heapSize] = e;
    heapify_up(heapSize);
    heapSize++;
}

EmergencyPatient* pop_emergency() {
    if (heapSize == 0) return NULL;
    EmergencyPatient *ret = (EmergencyPatient*) malloc(sizeof(EmergencyPatient));
    *ret = heapArr[0];
    heapSize--;
    if (heapSize > 0) {
        heapArr[0] = heapArr[heapSize];
        heapify_down(0);
    }
    return ret;
}

typedef struct Doctor {
    int id;
    char name[MAX_NAME];
    char specialist[MAX_NAME];
    struct Doctor *left, *right;
} Doctor;

Doctor *docRoot = NULL;

Doctor* create_doctor(int id, const char *name, const char *spec) {
    Doctor *d = (Doctor*) malloc(sizeof(Doctor));
    d->id = id;
    strncpy(d->name, name, MAX_NAME-1);
    d->name[MAX_NAME-1] = '\0';
    strncpy(d->specialist, spec, MAX_NAME-1);
    d->specialist[MAX_NAME-1] = '\0';
    d->left = d->right = NULL;
    return d;
}

Doctor* insert_doc(Doctor* root, Doctor* node) {
    node->left = NULL;
    node->right = NULL;
    if (!root) return node;
    Doctor *cur = root, *parent = NULL;
    while (cur) {
        parent = cur;
        if (node->id < cur->id) cur = cur->left;
        else cur = cur->right;
    }
    if (node->id < parent->id) parent->left = node;
    else parent->right = node;
    return root;
}

Doctor* search_doc(Doctor* root, int id) {
    if (!root) return NULL;
    if (id == root->id) return root;
    if (id < root->id) return search_doc(root->left, id);
    return search_doc(root->right, id);
}

void show_all_doctors_recursive(Doctor *root, int *row, int box_left, int box_top, int box_width, int box_height) {
    if (!root) return;
    show_all_doctors_recursive(root->left, row, box_left, box_top, box_width, box_height);
    int rel = (*row) - (box_top + 1);
    if (rel >= 0 && rel < box_height - 2) {
        int x = box_left + 2;
        gotoxy(x, *row);
        char line[200];
        snprintf(line, sizeof(line), "ID: %-3d  Name: %-15.18s  Specialist: %-17.18s",
                 root->id, root->name, root->specialist);
        printf("%s", line);
    }
    (*row)++;
    show_all_doctors_recursive(root->right, row, box_left, box_top, box_width, box_height);
}

Doctor* find_min_doctor(Doctor* root) {
    if (!root) return NULL;
    while (root->left) root = root->left;
    return root;
}

Doctor* delete_doctor_recursive(Doctor* root, int id, int *deleted) {
    if (!root) return NULL;
    if (id < root->id) root->left = delete_doctor_recursive(root->left, id, deleted);
    else if (id > root->id) root->right = delete_doctor_recursive(root->right, id, deleted);
    else {
        *deleted = 1;
        if (!root->left && !root->right)
        {
            free(root);
            return NULL;
        }
        if (!root->left)
        {
            Doctor *tmp = root->right;
            free(root);
            return tmp;
        }
        if (!root->right)
        {
            Doctor *tmp = root->left;
            free(root);
            return tmp;
        }
        Doctor *succ = find_min_doctor(root->right);
        root->id = succ->id;
        strncpy(root->name, succ->name, MAX_NAME-1);
        root->name[MAX_NAME-1]='\0';
        strncpy(root->specialist, succ->specialist, MAX_NAME-1);
        root->specialist[MAX_NAME-1] = '\0';
        root->right = delete_doctor_recursive(root->right, succ->id, deleted);
    }
    return root;
}

void add_normal_patient() {
    system("cls");
    fflush(stdin);
    int left, top;
    draw_centered_box(70,8,&left,&top);
    print_in_box_center(left, top-1, 70, 8, 0, "Add Normal Patient", 11);

    char name[MAX_NAME], prob[MAX_PROB];
    gotoxy(left + 4, top + 2);
    printf("Name    : ");
    fflush(stdin);
    scanf(" %[^\n]", name);
    gotoxy(left + 4, top + 3);
    printf("Problem : ");
    fflush(stdin);
    scanf(" %[^\n]", prob);

    enqueue_normal(name, prob);

    print_in_box_center(left, top, 70, 8, 5, "Normal patient added successfully (ID auto-generated).", 10);
    gotoxy(left + 4, top + 7);
    printf("Press any key to return...");
    getch();
}

void add_emergency_patient() {
    fflush(stdin);
    system("cls");
    int left, top;
    system("cls");
    fflush(stdin);
    draw_centered_box(80,9,&left,&top);
    print_in_box_center(left, top-1, 80, 9, 0, "Add Emergency Patient", 11);

    char name[MAX_NAME], prob[MAX_PROB];
    int pri;
    gotoxy(left + 4, top + 2);
    printf("Name     : ");
    fflush(stdin);
    scanf(" %[^\n]", name);
    gotoxy(left + 4, top + 3);
    printf("Problem  : ");
    fflush(stdin);
    scanf(" %[^\n]", prob);
    gotoxy(left + 4, top + 4);
    printf("Priority (1-100, high = more serious): ");
    if (scanf("%d", &pri) != 1)
    {
        pri = 50;
        while(getchar()!='\n');
    }

    push_emergency(name, prob, pri);

    print_in_box_center(left, top, 80, 9, 6, "Emergency patient added successfully.", 10);
    gotoxy(left + 4, top + 7);
    printf("Press any key to return...");
    getch();
}

void add_doctor() {
    fflush(stdin);
    system("cls");

    int left, top;
    draw_centered_box(70,10,&left,&top);
    print_in_box_center(left, top, 70, 10, 0, "Add Doctor", 11);

    int id;
    char name[MAX_NAME], spec[MAX_NAME];

    gotoxy(left + 4, top + 2);
    printf("Doctor ID    : ");
    if (scanf("%d", &id) != 1 || id <= 0) {
        while(getchar()!='\n');
        print_in_box_center(left, top, 70, 10, 6, "Invalid Doctor ID!", 12);
        getch();
        return;
    }

    if (search_doc(docRoot, id)) {
        print_in_box_center(left, top, 70, 10, 6, "Doctor ID already exists!", 12);
        getch();
        return;
    }

    gotoxy(left + 4, top + 3);
    printf("Doctor Name  : ");
    fflush(stdin);
    scanf(" %[^\n]", name);

    gotoxy(left + 4, top + 4);
    printf("Specialist   : ");
    fflush(stdin);
    scanf(" %[^\n]", spec);

    Doctor *d = create_doctor(id, name, spec);
    docRoot = insert_doc(docRoot, d);

    char msg[100];
    snprintf(msg, sizeof(msg), "Doctor added successfully. ID = %d", id);
    print_in_box_center(left, top, 70, 10, 7, msg, 10);

    gotoxy(left + 4, top + 9);
    printf("Press any key to return...");
    getch();
}


void show_normal_list() {
    fflush(stdin);
    system("cls");
    int left, top;
    draw_centered_box(100,20,&left,&top);
    print_in_box_center(left, top-1, 100, 20, 0, "Normal Patients Queue", 11);

    if (!qFront) {
        print_in_box_center(left, top, 100, 20, 8, "No normal patients in queue.", 12);
    } else {
        int row = top + 2;
        gotoxy(left + 3, top + 1);
        printf("ID   Name                 Problem");
        NormalPatient *cur = qFront;
        while (cur && row < top + 18) {
            gotoxy(left + 3, row);
            char line[200];
            snprintf(line, sizeof(line), "%-4d %-20.20s %-40.40s", cur->id, cur->name, cur->problem);
            printf("%s", line);
            cur = cur->next;
            row++;
        }
    }
    gotoxy(left + 35, top + 19);
    printf("Press any key to return...");
    getch();
}

void show_emergency_list() {
    fflush(stdin);
    system("cls");
    int left, top;
    draw_centered_box(100,20,&left,&top);
    print_in_box_center(left, top-1, 100, 20, 0, "Emergency Patients (Heap View)", 11);

    if (heapSize == 0) {
        print_in_box_center(left, top, 100, 20, 8, "No emergency patients.", 12);
    } else {
        gotoxy(left + 3, top + 1);
        printf("ID   Pri   Name                 Problem");
        int row = top + 2;
        for (int i=0; i<heapSize && row < top + 18; i++) {
            gotoxy(left + 3, row);
            char line[200];
            snprintf(line, sizeof(line), "%-4d %-5d %-20.20s %-40.40s",
                   heapArr[i].id, heapArr[i].priority, heapArr[i].name, heapArr[i].problem);
            printf("%s", line);
            row++;
        }
    }
    gotoxy(left + 35, top + 18);
    printf("Press any key to return...");
    getch();
}

void show_all_doctors() {
    fflush(stdin);
    system("cls");
    int left, top;
    draw_centered_box(100,20,&left,&top);
    print_in_box_center(left, top-1, 100, 20, 0, "Doctors List", 11);

    if (!docRoot) {
        print_in_box_center(left, top, 100, 20, 8, "No doctors available.", 12);
    } else {
        int row = top + 2;
        gotoxy(left + 3, top + 1);
        printf("ID      \tName                 Specialist");
        show_all_doctors_recursive(docRoot, &row, left, top, 100, 20);
    }
    gotoxy(left + 35, top + 18);
    printf("Press any key to return...");
    getch();
}

void search_doctor() {
    fflush(stdin);
    system("cls");
    int left, top;
    draw_centered_box(60,8,&left,&top);
    print_in_box_center(left, top-1, 60, 8, 0, "Search Doctor By ID", 11);

    int id;
    gotoxy(left + 4, top + 2);
    printf("Enter doctor ID: ");
    if (scanf("%d", &id) != 1)
    {
        while(getchar()!='\n');
        id = -1;
    }

    int left2, top2;
    draw_centered_box(60,8,&left2,&top2);
    if (id < 0) {
        system("cls");
        print_in_box_center(left2, top2, 60, 8, 3, "Invalid ID entered.", 12);
    } else {
        system("cls");
        Doctor *d = search_doc(docRoot, id);
        if (!d) print_in_box_center(left2, top2, 60, 8, 3, "Doctor not found.", 12);
        else {
            char buf[120];
            snprintf(buf, sizeof(buf), "ID: %d  Name: %.18s  Specialist: %.18s", d->id, d->name, d->specialist);
            print_in_box_center(left2, top2, 60, 8, 2, "Doctor Found!", 10);
            print_in_box_center(left2, top2, 60, 8, 3, buf, 7);
        }
    }
    gotoxy(left2 + 18, top2 + 7);
    printf("Press any key to return...");
    getch();
}

void delete_doctor() {
    fflush(stdin);
    system("cls");
    int left, top;
    draw_centered_box(60,8,&left,&top);
    print_in_box_center(left, top-1, 60, 8, 0, "Delete Doctor By ID", 11);

    int id;
    gotoxy(left + 4, top + 2);
    printf("Enter doctor ID: ");
    if (scanf("%d", &id) != 1)
    {
        while(getchar()!='\n');
        id = -1;
    }

    int deleted = 0;
    if (id >= 0) docRoot = delete_doctor_recursive(docRoot, id, &deleted);

    int left2, top2;
    draw_centered_box(60,6,&left2,&top2);
    if (deleted) {
        system("cls");
        char buf[60]; snprintf(buf, sizeof(buf), "Doctor with ID %d deleted successfully.", id);
        print_in_box_center(left2, top2, 60, 6, 1, buf, 10);
    } else {
        system("cls");
        print_in_box_center(left2, top2, 60, 6, 1, "Doctor not found.", 12);
    }
    gotoxy(left2 + 18, top2 + 3);
    printf("Press any key to return...");
    getch();
}

typedef struct Staff {
    int id;
    char name[50];
    char position[50];
    int height;
    struct Staff *left;
    struct Staff *right;
} Staff;

Staff *staffRoot = NULL;

int height_staff(Staff *n) { 
    return (n == NULL) ? 0 : n->height; 
}
int max_int(int a, int b) { 
    return (a > b) ? a : b; 
}

Staff* newStaffNode(int id, const char *name, const char *position) {
    Staff *node = (Staff*) malloc(sizeof(Staff));
    node->id = id;
    strncpy(node->name, name, sizeof(node->name)-1);
    node->name[sizeof(node->name)-1] = '\0';
    strncpy(node->position, position, sizeof(node->position)-1);
    node->position[sizeof(node->position)-1] = '\0';
    node->height = 1;
    node->left = node->right = NULL;
    return node;
}

Staff* rightRotateStaff(Staff* y) {
    Staff* x = y->left;
    Staff* T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = max_int(height_staff(y->left), height_staff(y->right)) + 1;
    x->height = max_int(height_staff(x->left), height_staff(x->right)) + 1;
    return x;
}

Staff* leftRotateStaff(Staff* x) {
    Staff* y = x->right;
    Staff* T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = max_int(height_staff(x->left), height_staff(x->right)) + 1;
    y->height = max_int(height_staff(y->left), height_staff(y->right)) + 1;
    return y;
}

int getBalanceStaff(Staff *n) {
    if (n == NULL) return 0;
    return height_staff(n->left) - height_staff(n->right);
}

Staff* insertStaffAVL(Staff* node, int id, const char *name, const char *position) {
    if (node == NULL)
        return newStaffNode(id, name, position);

    if (id < node->id)
        node->left = insertStaffAVL(node->left, id, name, position);
    else if (id > node->id)
        node->right = insertStaffAVL(node->right, id, name, position);
    else
        return node; 

    node->height = 1 + max_int(height_staff(node->left), height_staff(node->right));
    int balance = getBalanceStaff(node);

    if (balance > 1 && id < node->left->id)
        return rightRotateStaff(node);

    if (balance < -1 && id > node->right->id)
        return leftRotateStaff(node);

    if (balance > 1 && id > node->left->id) {
        node->left = leftRotateStaff(node->left);
        return rightRotateStaff(node);
    }
    if (balance < -1 && id < node->right->id) {
        node->right = rightRotateStaff(node->right);
        return leftRotateStaff(node);
    }

    return node;
}


void enter_new_staff() {
    fflush(stdin);
    system("cls");

    int left, top;
    draw_centered_box(60, 12, &left, &top);
    print_in_box_center(left, top - 1, 60, 12, 0, "Enter New Staff", 11);

    int id = 0;
    char name[50] = {0};
    char position[50] = {0};

    gotoxy(left + 3, top + 2);
    printf("ID: ");
    gotoxy(left + 3, top + 3);
    printf("Name: ");
    gotoxy(left + 3, top + 4);
    printf("Position: ");

    gotoxy(left + 8, top + 2);
    if (scanf("%d", &id) != 1) {
        gotoxy(left + 3, top + 6);
        print_in_box_center(left, top + 6, 60, 1, 8, "Invalid ID input.", 12);
        gotoxy(left + 20, top + 9);
        printf("Press any key to return...");
        getch();
        
        int c; while ((c = getchar()) != '\n' && c != EOF) { }
        return;
    }
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { } 

    gotoxy(left + 9, top + 3);
    if (fgets(name, sizeof(name), stdin) == NULL) name[0] = '\0';
    else {
        size_t len = strlen(name);
        if (len > 0 && name[len-1] == '\n') name[len-1] = '\0';
    }

    gotoxy(left + 13, top + 4);
    if (fgets(position, sizeof(position), stdin) == NULL) position[0] = '\0';
    else {
        size_t len = strlen(position);
        if (len > 0 && position[len-1] == '\n') position[len-1] = '\0';
    }
    
    fflush(stdin);
    while (name[0] == ' ') memmove(name, name+1, strlen(name));
    fflush(stdin);
    while (position[0] == ' ') memmove(position, position+1, strlen(position));
    fflush(stdin);

    if (id <= 0 || strlen(name) == 0) {
        gotoxy(left + 3, top + 6);
        print_in_box_center(left, top + 6, 60, 1, 8, "ID and Name are required (ID>0).", 12);
        gotoxy(left + 20, top + 10);
        printf("Press any key to return...");
        getch();
        return;
    }

    staffRoot = insertStaffAVL(staffRoot, id, name, position);

    gotoxy(left + 3, top + 6);
    print_in_box_center(left, top + 6, 60, 1, 2, "New staff added successfully.", 10);
    gotoxy(left + 20, top + 10);
    printf("Press any key to return...");
    getch();
}

void show_all_staff_recursive(Staff *node, int *row, int left, int top, int width, int height) {
    if (node == NULL) return;

    show_all_staff_recursive(node->left, row, left, top, width, height);

    int max_print_row = top + height - 3; 
    if (*row <= max_print_row) {
        gotoxy(left + 3, *row);
        printf("%-8d\t%-20s%s\n", node->id, node->name, node->position);
        (*row)++;
    } else {
        return;
    }

    show_all_staff_recursive(node->right, row, left, top, width, height);
}

void show_all_staff() {
    fflush(stdin);
    system("cls");
    int left, top;
    draw_centered_box(100, 20, &left, &top);
    print_in_box_center(left, top - 1, 100, 20, 0, "Staff List", 11);

    if (!staffRoot) {
        print_in_box_center(left, top, 100, 20, 8, "No staff available.", 12);
    } else {
        int row = top + 2;
        gotoxy(left + 3, top + 1);
        printf("ID      \tName                 Position");
        show_all_staff_recursive(staffRoot, &row, left, top, 100, 20);
    }
    gotoxy(left + 35, top + 19);
    printf("Press any key to return...");
    getch();
}

void assign_appointments() {
    fflush(stdin);
    system("cls");
    int left, top;
    draw_centered_box(100,20,&left,&top);
    print_in_box_center(left, top-1, 100, 20, 0, "Assigning Appointments", 11);

    int row = top + 2;
    gotoxy(left + 3, top + 1);
    printf("[EMERGENCY FIRST, THEN NORMAL PATIENTS]");

    while (heapSize > 0 && row < top + 18) {
        EmergencyPatient *e = pop_emergency();
        gotoxy(left + 3, row);
        char line[200];
        snprintf(line, sizeof(line), "[EMERGENCY] ID:%d Pri:%d Name:%.18s Problem:%.30s",
               e->id, e->priority, e->name, e->problem);
        printf("%s", line);
        free(e);
        row++;
    }

    while (qFront && row < top + 18) {
        NormalPatient *p = dequeue_normal();
        gotoxy(left + 3, row);
        char line[200];
        snprintf(line, sizeof(line), "[NORMAL]   ID: %d Name: %.18s Problem: %.30s", p->id, p->name, p->problem);
        printf("%s", line);
        free(p);
        row++;
    }

    if (row == top + 2) {
        print_in_box_center(left, top, 100, 20, 8, "No patients in system.", 12);
    }

    gotoxy(left + 35, top + 18); printf("All possible appointments assigned. Press any key...");
    getch();
}

void free_doctors(Doctor *root) {
    if (!root) return;
    free_doctors(root->left);
    free_doctors(root->right);
    free(root);
}

void free_normal_queue() {
    NormalPatient *cur = qFront;
    while (cur) {
        NormalPatient *tmp = cur;
        cur = cur->next;
        free(tmp);
    }
    qFront = qRear = NULL;
}

void clear_emergency_heap() {
    heapSize = 0;
}

void save_to_file(const char *filename) {
    fflush(stdin);
    system("cls");
    FILE *f = fopen(filename, "wb");
    if (!f) {
        int l,t; 
        draw_centered_box(60,6,&l,&t);
        print_in_box_center(l,t,60,6,1,"Failed to open file for saving.",12);
        getch();
        return;
    }

    fwrite(&nextNormalId, sizeof(int), 1, f);
    fwrite(&nextEmergencyId, sizeof(int), 1, f);
    int normalCount = 0;
    NormalPatient *cur = qFront;
    while (cur) 
    { 
        normalCount++; 
        cur = cur->next; 
    }
    fwrite(&normalCount, sizeof(int), 1, f);
    cur = qFront;
    while (cur) {
        fwrite(&cur->id, sizeof(int), 1, f);
        fwrite(cur->name, sizeof(char), MAX_NAME, f);
        fwrite(cur->problem, sizeof(char), MAX_PROB, f);
        cur = cur->next;
    }

    fwrite(&heapSize, sizeof(int), 1, f);
    for (int i=0;i<heapSize;i++) {
        fwrite(&heapArr[i].id, sizeof(int), 1, f);
        fwrite(&heapArr[i].priority, sizeof(int), 1, f);
        fwrite(heapArr[i].name, sizeof(char), MAX_NAME, f);
        fwrite(heapArr[i].problem, sizeof(char), MAX_PROB, f);
    }

    int docCount = 0;
    Doctor *stack[1000]; 
    int sp = 0; 
    Doctor *node = docRoot;
    while (sp || node) {
        while (node) 
        { 
            stack[sp++] = node; 
            node = node->left; 
        }
        node = stack[--sp];
        docCount++;
        node = node->right;
    }
    fwrite(&docCount, sizeof(int), 1, f);
    sp = 0; 
    node = docRoot;
    while (sp || node) {
        while (node) 
        { 
            stack[sp++] = node; 
            node = node->left; 
        }
        node = stack[--sp];
        fwrite(&node->id, sizeof(int), 1, f);
        fwrite(node->name, sizeof(char), MAX_NAME, f);
        fwrite(node->specialist, sizeof(char), MAX_NAME, f);
        node = node->right;
    }

    fclose(f);

    int l,t; 
    draw_centered_box(60,6,&l,&t);
    print_in_box_center(l,t,60,6,1,"Data saved successfully.",10);
    getch();
}

void load_from_file(const char *filename) {
    fflush(stdin);
    system("cls");
    FILE *f = fopen(filename, "rb");
    if (!f) {
        int l,t; 
        draw_centered_box(60,6,&l,&t);
        print_in_box_center(l,t,60,6,1,"Failed to open file for loading.",12);
        getch();
        return;
    }

    free_normal_queue();
    free_doctors(docRoot); 
    docRoot = NULL;
    clear_emergency_heap();

    fread(&nextNormalId, sizeof(int), 1, f);
    fread(&nextEmergencyId, sizeof(int), 1, f);

    int normalCount = 0; 
    fread(&normalCount, sizeof(int), 1, f);
    for (int i=0;i<normalCount;i++) {
        NormalPatient temp;
        fread(&temp.id, sizeof(int), 1, f);
        fread(temp.name, sizeof(char), MAX_NAME, f);
        fread(temp.problem, sizeof(char), MAX_PROB, f);
        NormalPatient *p = (NormalPatient*) malloc(sizeof(NormalPatient));
        *p = temp;
        p->next = NULL;
        if (!qFront) qFront = qRear = p;
        else { 
            qRear->next = p; 
            qRear = p; 
        }
    }

    fread(&heapSize, sizeof(int), 1, f);
    for (int i=0;i<heapSize;i++) {
        fread(&heapArr[i].id, sizeof(int), 1, f);
        fread(&heapArr[i].priority, sizeof(int), 1, f);
        fread(heapArr[i].name, sizeof(char), MAX_NAME, f);
        fread(heapArr[i].problem, sizeof(char), MAX_PROB, f);
    }

    int docCount = 0; 
    fread(&docCount, sizeof(int), 1, f);
    for (int i=0;i<docCount;i++) {
        int id; char name[MAX_NAME], spec[MAX_NAME];
        fread(&id, sizeof(int), 1, f);
        fread(name, sizeof(char), MAX_NAME, f);
        fread(spec, sizeof(char), MAX_NAME, f);
        Doctor *d = create_doctor(id, name, spec);
        docRoot = insert_doc(docRoot, d);
    }

    fclose(f);

    int l,t; 
    draw_centered_box(60,6,&l,&t);
    print_in_box_center(l,t,60,6,1,"Data loaded successfully.",10);
    getch();
}

void loadingScreen () {
    system("cls");
    int cols, rows;
    get_console_size(&cols,&rows);
    int mid = cols/2;
    gotoxy(mid-18, rows/2 - 2);
    printf("PLEASE WAIT, LOADING HOSPITAL SYSTEM . . .\n\n");
    gotoxy(mid-35, rows/2);
    for(int loading=0; loading<70 && loading < cols-10; loading++)
    {
        Sleep(10);
        setcolor(6);
        printf("%c",219);
    }
    setcolor(7);
    Sleep(150);
    system("cls");
}

void endScreen () {
    system("cls");
    int cols, rows;
    get_console_size(&cols,&rows);
    gotoxy((cols/2)-8, rows/2 - 2);
    printf("PLEASE WAIT . . .\n\n");
    gotoxy((cols/2)-35, rows/2);
    for(int loading=0; loading<70 && loading < cols-10; loading++)
    {
        Sleep(10);
        printf("%c",219);
    }
    Sleep(150);
    system("cls");

    int left, top;
    draw_centered_box(60,6,&left,&top);
    print_in_box_center(left, top, 60, 6, 1, "THANK YOU FOR USING HOSPITAL MANAGEMENT SYSTEM", 10);
    gotoxy(left + (60/2) - 10, top + 4);
    printf("Press any key to exit...");
    getch();
}

void show_main_menu() {
    int left, top;
    draw_centered_box(100,16,&left,&top);

    print_in_box_center(left, top, 100, 16, 0, "Hospital Patient & Doctor Management System", 4);

    gotoxy(left + 6, top + 2);
    setcolor(1);
    printf("1. Add Normal Patient");
    gotoxy(left + 6, top + 3);
    printf("2. Add Emergency Patient");
    gotoxy(left + 6, top + 4);
    printf("3. Show Normal Patient Queue");
    gotoxy(left + 6, top + 5);
    printf("4. Show Emergency Patient List");
    gotoxy(left + 6, top + 6);
    printf("9. Staff Enter");

    gotoxy(left + 56, top + 2);
    printf("5. Add Doctor");
    gotoxy(left + 56, top + 3);
    printf("6. Show All Doctors");
    gotoxy(left + 56, top + 4);
    printf("7. Search Doctor by ID");
    gotoxy(left + 56, top + 5);
    printf("8. Delete Doctor by ID");
    gotoxy(left + 56, top + 6);
    printf("10. Show All Avaiable Staff");

    gotoxy(left + 6, top + 7);
    setcolor(1);
    printf("11. Assign Appointments (Process All Patients)");

    gotoxy(left + 40, top + 9);
    setcolor(3);
    printf("12. Save Data to File");
    gotoxy(left + 40, top + 10);
    printf("13. Load Data from File");
    setcolor(7);

    gotoxy(left + 42, top + 13);
    setcolor(7);
    printf("0. Exit");

    gotoxy(left + 36, top + 14);
    printf("Enter your choice: ");
    setcolor(7);
}

int main() {
    int op;
    loadingScreen();

    do {
        system("cls");
        show_main_menu();
        if (scanf("%d", &op) != 1) {
            while(getchar()!='\n'); 
            op = -1;
        }

        switch(op) {
            case 1:
                add_normal_patient();
                break;
            case 2:
                add_emergency_patient();
                break;
            case 3:
                show_normal_list();
                break;
            case 4:
                show_emergency_list();
                break;
            case 5:
                add_doctor();
                break;
            case 6:
                show_all_doctors();
                break;
            case 7:
                search_doctor();
                break;
            case 8:
                delete_doctor();
                break;
            case 9:
                enter_new_staff();
                break;
            case 10:
                show_all_staff();
                break;
            case 11:
                assign_appointments();
                break;
            case 12:
                save_to_file("hospital_data.bin");
                break;
            case 13:
                load_from_file("hospital_data.bin");
                break;
            case 0:
                endScreen();
                break;
            default:
                {
                    fflush(stdin);
                    system("cls");
                    int left, top;
                    draw_centered_box(60,6,&left,&top);
                    print_in_box_center(left, top, 60, 6, 1, "Invalid choice! Please try again.", 12);
                    gotoxy(left + 18, top + 4);
                    printf("Press any key to return...");
                    getch();
                }
                break;
        }

    } while(op != 0);

    return 0;
}