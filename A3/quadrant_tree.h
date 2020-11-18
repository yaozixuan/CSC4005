/* quadrant_tree head file */
#pragma once

/* 
0--------------------
|         |         |
|    4    |    1    |
|         |         |
|         |         |
---------------------
|         |         |
|         |         |
|    3    |    2    |
|         |         |
---------------------
*/
enum quadrant{q_1, q_2, q_3, q_4};

struct body {
    double m;      
    double x, y;    
    double vx, vy;  
    double fx, fy;
    int active;
};

struct tree {
    double totalmass;
    double centerx, centery;
    double xmin, xmax;
    double ymin, ymax;
    double xmid, ymid;
    struct body * virtualbody;
    struct tree * q_1;
    struct tree * q_2;
    struct tree * q_3;
    struct tree * q_4;
};


enum quadrant Get_Quadrant(double x, double y, double x_min, double x_max, double y_min, double y_max) {

    double x_mid, y_mid;

    x_mid = 0.5*(x_min + x_max);
    y_mid = 0.5*(y_min + y_max);

    if (x < x_mid) {
        if (y < y_mid) {
            return q_4;
        } else {
            return q_3;
        }
    } else {
        if (y < y_mid) {
            return q_1;
        } else {
            return q_2;
        }
    }
}

struct tree *Create_TreeNode(struct body * virtualbody, double xmin, double xmax, double ymin, double ymax) {

    struct tree* TreeNode;
    TreeNode = malloc(sizeof(struct tree));

    TreeNode->totalmass = virtualbody->m;
    TreeNode->centerx = virtualbody->x;
    TreeNode->centery = virtualbody->y;
    TreeNode->xmin = xmin;
    TreeNode->xmax = xmax;
    TreeNode->ymin = ymin;
    TreeNode->ymax = ymax;
    TreeNode->xmid = 0.5*(xmin + xmax);
    TreeNode->ymid = 0.5*(ymin + ymax);
    TreeNode->virtualbody = virtualbody;
    TreeNode->q_1 = NULL;
    TreeNode->q_2 = NULL;
    TreeNode->q_3 = NULL;
    TreeNode->q_4 = NULL;
    
    return TreeNode;
}

void Insert_Body(struct body * insbody, struct tree * TreeNode) {
    
    // Update_Center_Mass(TreeNode, insbody);
    TreeNode->totalmass += insbody->m;
    TreeNode->centerx = (TreeNode->totalmass*TreeNode->centerx + insbody->m*insbody->x) / (TreeNode->totalmass + insbody->m);
    TreeNode->centery = (TreeNode->totalmass*TreeNode->centery + insbody->m*insbody->y) / (TreeNode->totalmass + insbody->m);

    /* virtualbody is not virtual, stores the leaf body */
    if (TreeNode->virtualbody != NULL) {
        /* If two bodies are exactly at the same point, merge them, otherwise, the program will recursively call Insert_Body to split them (Infinite recursive cakks) */
        if (TreeNode->virtualbody->x == insbody->x && TreeNode->virtualbody->y == insbody->y) {
            TreeNode->virtualbody->m += insbody->m;
            TreeNode->virtualbody->vx = (TreeNode->virtualbody->m*TreeNode->virtualbody->vx + insbody->m*insbody->vx) / (TreeNode->virtualbody->m + insbody->m);
            TreeNode->virtualbody->vy = (TreeNode->virtualbody->m*TreeNode->virtualbody->vy + insbody->m*insbody->vy) / (TreeNode->virtualbody->m + insbody->m);
            insbody->active = 0;
        }
        else {
            /* Move the real body to its corresponding quadrant */
            enum quadrant myquad;
            myquad = Get_Quadrant(TreeNode->virtualbody->x, TreeNode->virtualbody->y, TreeNode->xmin, TreeNode->xmax, TreeNode->ymin, TreeNode->ymax);

            if (myquad ==q_4) {
                TreeNode->q_4 = Create_TreeNode(TreeNode->virtualbody, TreeNode->xmin, TreeNode->xmid, TreeNode->ymin, TreeNode->ymid);
            } 
            else if (myquad == q_1) {
                TreeNode->q_1 = Create_TreeNode(TreeNode->virtualbody, TreeNode->xmid, TreeNode->xmax, TreeNode->ymin, TreeNode->ymid);
            } 
            else if (myquad == q_3) {
                TreeNode->q_3 = Create_TreeNode(TreeNode->virtualbody, TreeNode->xmin, TreeNode->xmid, TreeNode->ymid, TreeNode->ymax);
            } 
            else {
                TreeNode->q_2 = Create_TreeNode(TreeNode->virtualbody, TreeNode->xmid, TreeNode->xmax, TreeNode->ymid, TreeNode->ymax);
            }

            TreeNode->virtualbody = NULL;
        }
    } 
    /* virtualbody is actually virtual, stores no real body */
    if (TreeNode->virtualbody == NULL) {
        enum quadrant insquad;
        insquad = Get_Quadrant(insbody->x, insbody->y, TreeNode->xmin, TreeNode->xmax, TreeNode->ymin, TreeNode->ymax);
        if (insquad == q_4) {
            if (TreeNode->q_4 == NULL) {
                TreeNode->q_4 = Create_TreeNode(insbody, TreeNode->xmin, TreeNode->xmid, TreeNode->ymin, TreeNode->ymid);
            } 
            else {
                Insert_Body(insbody, TreeNode->q_4);
            }
        } 
        else if (insquad == q_1) {
            if (TreeNode->q_1 == NULL) {
                TreeNode->q_1 = Create_TreeNode(insbody, TreeNode->xmid, TreeNode->xmax, TreeNode->ymin, TreeNode->ymid);
            } 
            else {
                Insert_Body(insbody, TreeNode->q_1);
            }
        } 
        else if (insquad == q_3) {
            if (TreeNode->q_3 == NULL) {
                TreeNode->q_3 = Create_TreeNode(insbody, TreeNode->xmin, TreeNode->xmid, TreeNode->ymid, TreeNode->ymax);
            } 
            else {
                Insert_Body(insbody, TreeNode->q_3);
            }
        } 
        else if (insquad == q_2) {
            if (TreeNode->q_2 == NULL) {
                TreeNode->q_2 = Create_TreeNode(insbody, TreeNode->xmid, TreeNode->xmax, TreeNode->ymid, TreeNode->ymax);
            } 
            else {
                Insert_Body(insbody, TreeNode->q_2);
            }
        }
    }
}

void Calculate_force(struct tree * TreeNode, struct body * bodyp, double G, double threshold) {
    double dx, dy, d, fx, fy, s;

    dx = TreeNode->centerx - bodyp->x;
    dy = TreeNode->centery - bodyp->y;
    d = sqrt(pow(dx, 2) + pow(dy, 2));
    s = TreeNode->xmax - TreeNode->xmin;

    if ((s/d < threshold || TreeNode->virtualbody != NULL) && bodyp != TreeNode->virtualbody) {

        fx = G*TreeNode->totalmass*bodyp->m*dx / pow(d+3, 3);
        fy = G*TreeNode->totalmass*bodyp->m*dy / pow(d+3, 3);

        bodyp->fx += fx;
        bodyp->fy += fy;
    } 
    else {
        if (TreeNode->q_4 != NULL) Calculate_force(TreeNode->q_4, bodyp, G, threshold);
        if (TreeNode->q_1 != NULL) Calculate_force(TreeNode->q_1, bodyp, G, threshold);
        if (TreeNode->q_3 != NULL) Calculate_force(TreeNode->q_3, bodyp, G, threshold);
        if (TreeNode->q_2 != NULL) Calculate_force(TreeNode->q_2, bodyp, G, threshold);
    }
}

void Destroy_Tree(struct tree * TreeNode) {
    if (TreeNode != NULL) {
        if (TreeNode->q_4 != NULL) Destroy_Tree(TreeNode->q_4);
        if (TreeNode->q_1 != NULL) Destroy_Tree(TreeNode->q_1);
        if (TreeNode->q_3 != NULL) Destroy_Tree(TreeNode->q_3);
        if (TreeNode->q_2 != NULL) Destroy_Tree(TreeNode->q_2);
        free(TreeNode);
    }
}