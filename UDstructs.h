#include <stdio.h>
#include <stdlib.h>
#include "apex_cpu.h"

/* Physical Registers */

typedef struct regs_physicalregs_map
{
    int rf_code;
    int prf_code;
} regs_physicalregs_map;

typedef struct hasher
{
    regs_physicalregs_map data;
    struct hasher *next;
} hasher;

regs_physicalregs_map renametable;
hasher *rfprf;
typedef void (*callback2)(hasher *data);

hasher *create_hasher(regs_physicalregs_map data, hasher *next)
{
    hasher *new_preg = (hasher *)malloc(sizeof(hasher));
    if (new_preg == NULL)
    {
        printf("Error creating a new hasher.\n");
        exit(0);
    }
    new_preg->data = data;
    new_preg->next = next;

    return new_preg;
}

hasher *remove_back_hasher(hasher *head)
{
    if (head == NULL)
        return NULL;

    hasher *cursor = head;
    hasher *back = NULL;
    while (cursor->next != NULL)
    {
        back = cursor;
        cursor = cursor->next;
    }

    if (back != NULL)
        back->next = NULL;

    /* if this is the last node in the list*/
    if (cursor == head)
        head = NULL;

    free(cursor);

    return head;
}
hasher *remove_front_hasher(hasher *head)
{
    if (head == NULL)
        return NULL;
    hasher *front = head;
    head = head->next;
    front->next = NULL;
    /* is this the last node in the list */
    if (front == head)
        head = NULL;
    free(front);
    return head;
}

hasher *remove_any_hasher(hasher *head, hasher *nd)
{
    if (nd == NULL)
        return NULL;
    /* if the node is the first node */
    if (nd == head)
        return remove_front_hasher(head);

    /* if the node is the last node */
    if (nd->next == NULL)
        return remove_back_hasher(head);

    /* if the node is in the middle */
    hasher *cursor = head;
    while (cursor != NULL)
    {
        if (cursor->next == nd)
            break;
        cursor = cursor->next;
    }

    if (cursor != NULL)
    {
        hasher *tmp = cursor->next;
        cursor->next = tmp->next;
        tmp->next = NULL;
        free(tmp);
    }
    return head;
}

hasher *prepend_hasher(hasher *head, regs_physicalregs_map data)
{
    hasher *new_preg = create_hasher(data, head);
    head = new_preg;
    return head;
}

hasher *append_hasher(hasher *head, regs_physicalregs_map data)
{
    if (head == NULL)
        return NULL;
    /* go to the last hasher */
    hasher *cursor = head;
    while (cursor->next != NULL)
        cursor = cursor->next;

    /* create_physical_register1 a new hasher */
    hasher *new_preg = create_hasher(data, NULL);
    cursor->next = new_preg;

    return head;
}

hasher *enqueue_physical_register(hasher *head, regs_physicalregs_map data)
{
    if (head == NULL)
    {
        head = prepend_hasher(head, data);
    }
    else
    {
        head = append_hasher(head, data);
    }
    return head;
}

void traverse_physical_register(hasher *head, callback2 f)
{
    hasher *cursor = head;
    while (cursor != NULL)
    {
        f(cursor);
        cursor = cursor->next;
    }
}

hasher *dequeue_physical_register(hasher *head)
{
    if (head == NULL)
        return NULL;
    hasher *front = head;
    head = head->next;
    front->next = NULL;
    /* is this the last hasher in the list */
    if (front == head)
        head = NULL;
    free(front);
    return head;
}

hasher *search_physical_register2(hasher *head, int data)
{

    hasher *cursor = head;
    while (cursor != NULL)
    {
        if (cursor->data.rf_code == data)
            return cursor;
        cursor = cursor->next;
    }
    return NULL;
}

int search_physical_register(hasher *head, int data)
{

    hasher *cursor = head;
    while (cursor != NULL)
    {
        if (cursor->data.rf_code == data)
            return cursor->data.prf_code;
        cursor = cursor->next;
    }
    return PREGS_FILE_SIZE;
}

int search_physical_register1(hasher *head, int data)
{

    hasher *cursor = head;
    while (cursor != NULL)
    {
        if (cursor->data.rf_code == data)
            return cursor->data.prf_code;
        cursor = cursor->next;
    }
    return -1;
}

int count_physical_register(hasher *head)
{
    hasher *cursor = head;
    int c = 0;
    while (cursor != NULL)
    {
        c++;
        cursor = cursor->next;
    }
    return c;
}

regs_physicalregs_map search_physical_register_AtIndex(hasher *head, int index)
{

    hasher *cursor = head;
    int i = 0;
    while (i != index)
    {

        cursor = cursor->next;
        i++;
    }
    return cursor->data;
}

void dispose_physical_register(hasher *head)
{
    hasher *cursor, *tmp;

    if (head != NULL)
    {
        cursor = head->next;
        head->next = NULL;
        while (cursor != NULL)
        {
            tmp = cursor->next;
            free(cursor);
            cursor = tmp;
        }
    }
}

/* Register Renaming */
typedef struct physical_regs
{
    int data;
    struct physical_regs *next;
} physical_regs;

physical_regs *phead;
physical_regs *ptail;
typedef void (*callback1)(physical_regs *data);

physical_regs *create_physical_register1(int data, physical_regs *next)
{
    physical_regs *new_pregs = (physical_regs *)malloc(sizeof(physical_regs));
    if (new_pregs == NULL)
    {
        printf("Error creating a new preg.\n");
        exit(0);
    }
    new_pregs->data = data;
    new_pregs->next = next;

    return new_pregs;
}

physical_regs *prependIntoPreg(physical_regs *head, int data)
{
    physical_regs *new_preg = create_physical_register1(data, head);
    head = new_preg;
    return head;
}

physical_regs *append1(physical_regs *head, int data)
{
    if (head == NULL)
        return NULL;
    physical_regs *cursor = head;
    while (cursor->next != NULL)
        cursor = cursor->next;

    physical_regs *new_preg = create_physical_register1(data, NULL);
    cursor->next = new_preg;

    return head;
}

physical_regs *enqueueReg(physical_regs *head, int data)
{
    if (head == NULL)
    {
        head = prependIntoPreg(head, data);
    }
    else
    {
        head = append1(head, data);
    }
    return head;
}

void traverse1(physical_regs *head, callback1 f)
{
    physical_regs *cursor = head;
    while (cursor != NULL)
    {
        f(cursor);
        cursor = cursor->next;
    }
}

physical_regs *dequeueReg(physical_regs *head)
{
    if (head == NULL)
        return NULL;
    physical_regs *front = head;
    head = head->next;
    front->next = NULL;
    if (front == head)
        head = NULL;
    free(front);
    return head;
}

int countReg(physical_regs *head)
{
    physical_regs *cursor = head;
    int c = 0;
    while (cursor != NULL)
    {
        c++;
        cursor = cursor->next;
    }
    return c;
}

int searchAtIndexReg(physical_regs *head, int index)
{

    physical_regs *cursor = head;
    int i = 0;
    while (i != index)
    {

        cursor = cursor->next;
        i++;
    }
    return cursor->data;
}

void disposeReg(physical_regs *head)
{
    physical_regs *cursor, *tmp;

    if (head != NULL)
    {
        cursor = head->next;
        head->next = NULL;
        while (cursor != NULL)
        {
            tmp = cursor->next;
            free(cursor);
            cursor = tmp;
        }
    }
}

typedef struct node
{
    CPU_Stage data;
    struct node *next;
} node;

typedef struct forwarding_node
{
    int rd;
    int pd;
    int pd_value;
} forwarding_node;

typedef struct forwarding_bus
{
    forwarding_node data;
    struct forwarding_bus *next;
} forwarding_bus;

/* BTB Struct*/
typedef struct BTB
{
    int active_flag;
    int inst_pc;
    int computed_address;
    int taken;
} BTB;

typedef struct btb_buffer
{
    BTB data;
    struct btb_buffer *next;
} btb_buffer;

node *iqhead;
node *lsqhead;
node *robhead;
btb_buffer *btb_head;

typedef void (*callback3)(btb_buffer *data);

btb_buffer *insert_address_btb(btb_buffer *head, CPU_Stage data, int res)
{
    btb_buffer *cursor = head;
    if (cursor != NULL)
    {
        while (cursor != NULL)
        {
            if (cursor->data.inst_pc == data.pc)
            {
                cursor->data.computed_address = data.result_buffer;
                cursor->data.active_flag = 1;
                cursor->data.taken = res;
                break;
            }
        }
    }
    return head;
}

btb_buffer *create_btb(BTB data, btb_buffer *next)
{
    btb_buffer *new_node = (btb_buffer *)malloc(sizeof(btb_buffer));
    if (new_node == NULL)
    {
        printf("Error creating a new btb_buffer.\n");
        exit(0);
    }
    new_node->data = data;
    new_node->next = next;

    return new_node;
}

btb_buffer *prepend_btb(btb_buffer *head, BTB data)
{
    btb_buffer *new_node = create_btb(data, head);
    head = new_node;
    return head;
}

btb_buffer *append_btb(btb_buffer *head, BTB data)
{
    if (head == NULL)
        return NULL;
    /* go to the last node */
    btb_buffer *cursor = head;
    while (cursor->next != NULL)
        cursor = cursor->next;

    /* create a new node */
    btb_buffer *new_node = create_btb(data, NULL);
    cursor->next = new_node;

    return head;
}

btb_buffer *enqueue_btb(btb_buffer *head, BTB data)
{
    if (head == NULL)
    {
        head = prepend_btb(head, data);
    }
    else
    {
        head = append_btb(head, data);
    }
    return head;
}

void traverse_btb(btb_buffer *head, callback3 f)
{
    btb_buffer *cursor = head;
    while (cursor != NULL)
    {
        f(cursor);
        cursor = cursor->next;
    }
}

btb_buffer *dequeue_btb(btb_buffer *head)
{
    if (head == NULL)
        return NULL;
    btb_buffer *front = head;
    head = head->next;
    front->next = NULL;
    /* is this the last node in the list */
    if (front == head)
        head = NULL;
    free(front);
    return head;
}

int count_btb(btb_buffer *head)
{
    btb_buffer *cursor = head;
    int c = 0;
    while (cursor != NULL)
    {
        c++;
        cursor = cursor->next;
    }
    return c;
}

BTB searchAtIndex_BTB(btb_buffer *head, int index)
{

    btb_buffer *cursor = head;
    int i = 0;
    while (i != index)
    {

        cursor = cursor->next;
        i++;
    }
    return cursor->data;
}

btb_buffer *remove_back_btb(btb_buffer *head)
{
    if (head == NULL)
        return NULL;

    btb_buffer *cursor = head;
    btb_buffer *back = NULL;
    while (cursor->next != NULL)
    {
        back = cursor;
        cursor = cursor->next;
    }

    if (back != NULL)
        back->next = NULL;

    /* if this is the last node in the list*/
    if (cursor == head)
        head = NULL;

    free(cursor);

    return head;
}
btb_buffer *remove_front_btb(btb_buffer *head)
{
    if (head == NULL)
        return NULL;
    btb_buffer *front = head;
    head = head->next;
    front->next = NULL;
    /* is this the last node in the list */
    if (front == head)
        head = NULL;
    free(front);
    return head;
}
btb_buffer *remove_any_btb(btb_buffer *head, btb_buffer *nd)
{
    if (nd == NULL)
        return NULL;
    /* if the node is the first node */
    if (nd == head)
        return remove_front_btb(head);

    /* if the node is the last node */
    if (nd->next == NULL)
        return remove_back_btb(head);

    /* if the node is in the middle */
    btb_buffer *cursor = head;
    while (cursor != NULL)
    {
        if (cursor->next == nd)
            break;
        cursor = cursor->next;
    }

    if (cursor != NULL)
    {
        btb_buffer *tmp = cursor->next;
        cursor->next = tmp->next;
        tmp->next = NULL;
        free(tmp);
    }
    return head;
}

void dispose_btb(btb_buffer *head)
{
    btb_buffer *cursor, *tmp;

    if (head != NULL)
    {
        cursor = head->next;
        head->next = NULL;
        while (cursor != NULL)
        {
            tmp = cursor->next;
            free(cursor);
            cursor = tmp;
        }
    }
}

// ====================================================

// typedef void (*callback)(forwarding_bus *data);

// forwarding_node *create_forwarding_node(forwarding_node data, forwarding_bus *next)
// {
//     forwarding_bus *new_node = (forwarding_bus *)malloc(sizeof(forwarding_bus));
//     if (new_node == NULL)
//     {
//         printf("Error creating a new node.\n");
//         exit(0);
//     }
//     new_node->data = data;
//     new_node->next = next;

//     return new_node;
// }

// forwarding_bus *prepend_forwarding_node(forwarding_bus *head, forwarding_node data)
// {
//     forwarding_bus *new_node = create_forwarding_node(data, head);
//     head = new_node;
//     return head;
// }

// forwarding_bus *append_forwarding_node(forwarding_bus *head, forwarding_node data)
// {
//     if (head == NULL)
//         return NULL;
//     /* go to the last node */
//     forwarding_bus *cursor = head;
//     while (cursor->next != NULL)
//         cursor = cursor->next;

//     /* create a new node */
//     forwarding_bus *new_node = create_forwarding_node(data, NULL);
//     cursor->next = new_node;

//     return head;
// }

// forwarding_bus *enqueue_forwarding_node(forwarding_bus *head, forwarding_node data)
// {
//     if (head == NULL)
//     {
//         head = prepend_forwarding_node(head, data);
//     }
//     else
//     {
//         head = append_forwarding_node(head, data);
//     }
//     return head;
// }

// void traverse_forwarding_node(forwarding_bus *head, callback f)
// {
//     forwarding_bus *cursor = head;
//     while (cursor != NULL)
//     {
//         f(cursor);
//         cursor = cursor->next;
//     }
// }

// forwarding_bus *dequeue_forwarding_node(forwarding_bus *head)
// {
//     if (head == NULL)
//         return NULL;
//     forwarding_bus *front = head;
//     head = head->next;
//     front->next = NULL;
//     /* is this the last node in the list */
//     if (front == head)
//         head = NULL;
//     free(front);
//     return head;
// }

// int count(forwarding_bus *head)
// {
//     forwarding_bus *cursor = head;
//     int c = 0;
//     while (cursor != NULL)
//     {
//         c++;
//         cursor = cursor->next;
//     }
//     return c;
// }

// forwarding_node searchAtIndex_forwarding_node(forwarding_bus *head, int index)
// {

//     forwarding_bus *cursor = head;
//     int i = 0;
//     while (i != index)
//     {

//         cursor = cursor->next;
//         i++;
//     }
//     return cursor->data;
// }

// forwarding_bus *remove_back_forwarding_node(forwarding_bus *head)
// {
//     if (head == NULL)
//         return NULL;

//     forwarding_bus *cursor = head;
//     forwarding_bus *back = NULL;
//     while (cursor->next != NULL)
//     {
//         back = cursor;
//         cursor = cursor->next;
//     }

//     if (back != NULL)
//         back->next = NULL;

//     /* if this is the last node in the list*/
//     if (cursor == head)
//         head = NULL;

//     free(cursor);

//     return head;
// }
// forwarding_bus *remove_front_forwarding_node(forwarding_bus *head)
// {
//     if (head == NULL)
//         return NULL;
//     forwarding_bus *front = head;
//     head = head->next;
//     front->next = NULL;
//     /* is this the last node in the list */
//     if (front == head)
//         head = NULL;
//     free(front);
//     return head;
// }
// forwarding_bus *remove_any_forwarding_node(forwarding_bus *head, forwarding_bus *nd)
// {
//     if (nd == NULL)
//         return NULL;
//     /* if the node is the first node */
//     if (nd == head)
//         return remove_front_forwarding_node(head);

//     /* if the node is the last node */
//     if (nd->next == NULL)
//         return remove_back_forwarding_node(head);

//     /* if the node is in the middle */
//     forwarding_bus *cursor = head;
//     while (cursor != NULL)
//     {
//         if (cursor->next == nd)
//             break;
//         cursor = cursor->next;
//     }

//     if (cursor != NULL)
//     {
//         forwarding_bus *tmp = cursor->next;
//         cursor->next = tmp->next;
//         tmp->next = NULL;
//         free(tmp);
//     }
//     return head;
// }

// void dispose_forwarding_node(forwarding_bus *head)
// {
//     forwarding_bus *cursor, *tmp;

//     if (head != NULL)
//     {
//         cursor = head->next;
//         head->next = NULL;
//         while (cursor != NULL)
//         {
//             tmp = cursor->next;
//             free(cursor);
//             cursor = tmp;
//         }
//     }
// }

// =====================================================
typedef void (*callback)(node *data);

node *create_node(CPU_Stage data, node *next)
{
    node *new_node = (node *)malloc(sizeof(node));
    if (new_node == NULL)
    {
        printf("Error creating a new node.\n");
        exit(0);
    }
    new_node->data = data;
    new_node->next = next;

    return new_node;
}

node *prepend(node *head, CPU_Stage data)
{
    node *new_node = create_node(data, head);
    head = new_node;
    return head;
}

node *append(node *head, CPU_Stage data)
{
    if (head == NULL)
        return NULL;
    /* go to the last node */
    node *cursor = head;
    while (cursor->next != NULL)
        cursor = cursor->next;

    /* create_node a new node */
    node *new_node = create_node(data, NULL);
    cursor->next = new_node;

    return head;
}

node *enqueue(node *head, CPU_Stage data)
{
    if (head == NULL)
    {
        head = prepend(head, data);
    }
    else
    {
        head = append(head, data);
    }
    return head;
}

void traverse(node *head, callback f)
{
    node *cursor = head;
    while (cursor != NULL)
    {
        f(cursor);
        cursor = cursor->next;
    }
}

node *dequeue(node *head)
{
    if (head == NULL)
        return NULL;
    node *front = head;
    head = head->next;
    front->next = NULL;
    /* is this the last node in the list */
    if (front == head)
        head = NULL;
    free(front);
    return head;
}

int count(node *head)
{
    node *cursor = head;
    int c = 0;
    while (cursor != NULL)
    {
        c++;
        cursor = cursor->next;
    }
    return c;
}

CPU_Stage searchAtIndex(node *head, int index)
{

    node *cursor = head;
    int i = 0;
    while (i != index)
    {

        cursor = cursor->next;
        i++;
    }
    return cursor->data;
}

node *remove_back(node *head)
{
    if (head == NULL)
        return NULL;

    node *cursor = head;
    node *back = NULL;
    while (cursor->next != NULL)
    {
        back = cursor;
        cursor = cursor->next;
    }

    if (back != NULL)
        back->next = NULL;

    /* if this is the last node in the list*/
    if (cursor == head)
        head = NULL;

    free(cursor);

    return head;
}
node *remove_front(node *head)
{
    if (head == NULL)
        return NULL;
    node *front = head;
    head = head->next;
    front->next = NULL;
    /* is this the last node in the list */
    if (front == head)
        head = NULL;
    free(front);
    return head;
}
node *remove_any(node *head, node *nd)
{
    if (nd == NULL)
        return NULL;
    /* if the node is the first node */
    if (nd == head)
        return remove_front(head);

    /* if the node is the last node */
    if (nd->next == NULL)
        return remove_back(head);

    /* if the node is in the middle */
    node *cursor = head;
    while (cursor != NULL)
    {
        if (cursor->next == nd)
            break;
        cursor = cursor->next;
    }

    if (cursor != NULL)
    {
        node *tmp = cursor->next;
        cursor->next = tmp->next;
        tmp->next = NULL;
        free(tmp);
    }
    return head;
}

void dispose(node *head)
{
    node *cursor, *tmp;

    if (head != NULL)
    {
        cursor = head->next;
        head->next = NULL;
        while (cursor != NULL)
        {
            tmp = cursor->next;
            free(cursor);
            cursor = tmp;
        }
    }
}
