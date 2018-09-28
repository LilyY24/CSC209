#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hcq.h"
#define INPUT_BUFFER_SIZE 256

/*
 * Return the number of students for a given course.
 */
int find_students_waiting(Course *course, Student *stu_list);

/*
 * Return the number of students currently being helped by TAs.
 */
int find_students_being_helped(Course *course, struct ta *ta_lst);

/*
 * Return whether the given student is in queue (with any course)
 */
int is_in_queue(Student *stu_list, Student *stu);

/**
 * Return whethere course_code is in courses; 
 */
int has_course(Course *courses, char *course_code, int num_course);

/*
 * Remove remove from student linked list;
 */
void remove_student(Student** student, Student* remove);

/*
 * Return the number of student waiting for the course
 */
int get_waiting_num(Course *courses, int num_courses, Course *course, Student* students);

/*
 * Return a pointer to the struct student with name stu_name
 * or NULL if no student with this name exists in the stu_list
 */
Student *find_student(Student *stu_list, char *student_name) {
    Student *cur = stu_list;
    while (cur != NULL){
        if (strcmp(cur->name, student_name) == 0){
            return cur;
        }
    }
    return NULL;
}



/*   Return a pointer to the ta with name ta_name or NULL
 *   if no such TA exists in ta_list. 
 */
Ta *find_ta(Ta *ta_list, char *ta_name) {
    Ta *cur = ta_list;
    while (cur != NULL){
        if (strcmp(cur->name, ta_name) == 0){
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}


/*  Return a pointer to the course with this code in the course list
 *  or NULL if there is no course in the list with this code.
 */
Course *find_course(Course *courses, int num_courses, char *course_code) {
    for (int i = 0; i < num_courses; i++){
        if (strcmp(courses[i].code, course_code) == 0){
            return &(courses[i]);
        }
    }
    return NULL;
}
    

/* Add a student to the queue with student_name and a question about course_code.
 * if a student with this name already has a question in the queue (for any
   course), return 1 and do not create the student.
 * If course_code does not exist in the list, return 2 and do not create
 * the student struct.
 * For the purposes of this assignment, don't check anything about the 
 * uniqueness of the name. 
 */
int add_student(Student **stu_list_ptr, char *student_name, char *course_code,
    Course *course_array, int num_courses) {
    if (!has_course(course_array, course_code, num_courses)){
        return 2;
    }
    Student *student = (Student*)malloc(sizeof(Student));
    student->name = (char*)malloc(sizeof(char)*strlen(student_name));
    strcpy(student->name, student_name);
    student->course = (Course*)malloc(sizeof(Course));
    student->course = find_course(course_array, num_courses, course_code);
    student->next_overall = (Student*)malloc(sizeof(Student));
    student->next_course = (Student*)malloc(sizeof(Student));
    student->arrival_time = (time_t*)malloc(sizeof(time_t));
    student->next_course = NULL;
    student->next_overall = NULL;
    *(student->arrival_time) = time(NULL);
    Student *cur = *stu_list_ptr;
    if (cur == NULL){
        *stu_list_ptr = student;
        return 0;
    }
    Student *last = NULL; 
    Student *last_course = NULL;
    while(cur != NULL){
        if (strcmp(cur->name, student_name) == 0){
            return 1;
        }
        if (strcmp(course_code, cur->course->code) == 0){
            last_course = cur;
        }
        last = cur;
        cur = cur->next_overall;
    }
    last->next_overall = student;
    if (last_course != NULL){
        last_course->next_course = student;
    }
    return 0;
}


/* Student student_name has given up waiting and left the help centre
 * before being called by a Ta. Record the appropriate statistics, remove
 * the student from the queues and clean up any no-longer-needed memory.
 *
 * If there is no student by this name in the stu_list, return 1.
 */
int give_up_waiting(Student **stu_list_ptr, char *student_name) {
    if (!is_in_queue(*stu_list_ptr, find_student(*stu_list_ptr, student_name))){
        return 1;
    }
    Student *student = find_student(*stu_list_ptr, student_name);
    student->course->wait_time += difftime(time(NULL), *(student->arrival_time));
    student->course->bailed++;
    remove_student(stu_list_ptr, student);
    free(student);
    return 0;
}

/* Create and prepend Ta with ta_name to the head of ta_list. 
 * For the purposes of this assignment, assume that ta_name is unique
 * to the help centre and don't check it.
 */
void add_ta(Ta **ta_list_ptr, char *ta_name) {
    // first create the new Ta struct and populate
    Ta *new_ta = malloc(sizeof(Ta));
    if (new_ta == NULL) {
       perror("malloc for TA");
       exit(1);
    }
    new_ta->name = malloc(strlen(ta_name)+1);
    if (new_ta->name  == NULL) {
       perror("malloc for TA name");
       exit(1);
    }
    strcpy(new_ta->name, ta_name);
    new_ta->current_student = NULL;

    // insert into front of list
    new_ta->next = *ta_list_ptr;
    *ta_list_ptr = new_ta;
}

/* The TA ta is done with their current student. 
 * Calculate the stats (the times etc.) and then 
 * free the memory for the student. 
 * If the TA has no current student, do nothing.
 */
void release_current_student(Ta *ta) {
    if (ta->current_student == NULL){
        return;
    }
    Student *cur = ta->current_student;
    cur->course->helped++;
    cur->course->help_time += difftime(time(NULL), *(cur->arrival_time));
    free(cur);
    ta->current_student = NULL;
}

/* Remove this Ta from the ta_list and free the associated memory with
 * both the Ta we are removing and the current student (if any).
 * Return 0 on success or 1 if this ta_name is not found in the list
 */
int remove_ta(Ta **ta_list_ptr, char *ta_name) {
    Ta *head = *ta_list_ptr;
    if (head == NULL) {
        return 1;
    } else if (strcmp(head->name, ta_name) == 0) {
        // TA is at the head so special case
        *ta_list_ptr = head->next;
        release_current_student(head);
        // memory for the student has been freed. Now free memory for the TA.
        free(head->name);
        free(head);
        return 0;
    }
    while (head->next != NULL) {
        if (strcmp(head->next->name, ta_name) == 0) {
            Ta *ta_tofree = head->next;
            //  We have found the ta to remove, but before we do that 
            //  we need to finish with the student and free the student.
            //  You need to complete this helper function
            release_current_student(ta_tofree);

            head->next = head->next->next;
            // memory for the student has been freed. Now free memory for the TA.
            free(ta_tofree->name);
            free(ta_tofree);
            return 0;
        }
        head = head->next;
    }
    // if we reach here, the ta_name was not in the list
    return 1;
}






/* TA ta_name is finished with the student they are currently helping (if any)
 * and are assigned to the next student in the full queue. 
 * If the queue is empty, then TA ta_name simply finishes with the student 
 * they are currently helping, records appropriate statistics, 
 * and sets current_student for this TA to NULL.
 * If ta_name is not in ta_list, return 1 and do nothing.
 */
int take_next_overall(char *ta_name, Ta *ta_list, Student **stu_list_ptr) {
    Ta *ta = find_ta(ta_list, ta_name);
    if (ta == NULL){
        return 1;
    }
    if (ta->current_student != NULL){
        ta->current_student->course->helped++;
        ta->current_student->course->help_time += difftime(time(NULL)
        , *(ta->current_student->arrival_time));
        free(ta->current_student);
    }
    if (*stu_list_ptr == NULL){
        return 0;
    }
    ta->current_student = *stu_list_ptr;
    *stu_list_ptr = (**stu_list_ptr).next_overall;
    return 0;
}



/* TA ta_name is finished with the student they are currently helping (if any)
 * and are assigned to the next student in the course with this course_code. 
 * If no student is waiting for this course, then TA ta_name simply finishes 
 * with the student they are currently helping, records appropriate statistics,
 * and sets current_student for this TA to NULL.
 * If ta_name is not in ta_list, return 1 and do nothing.
 * If course is invalid return 2, but finish with any current student. 
 */
int take_next_course(char *ta_name, Ta *ta_list, Student **stu_list_ptr, char *course_code, Course *courses, int num_courses) {
    Ta *ta = find_ta(ta_list, ta_name);
    if (ta == NULL){
        return 1;
    }
    if (ta->current_student != NULL){
        ta->current_student->course->helped++;
        ta->current_student->course->help_time += difftime(time(NULL)
        , *(ta->current_student->arrival_time));
        free(ta->current_student);
    }
    if (*stu_list_ptr == NULL){
        return 0;
    }
    Course *course = NULL;
    for (int i = 0; i < num_courses; i++){
        if (strcmp(courses[i].code, course_code) == 0){
            course = &courses[i];
            break;
        }
    }
    if (course == NULL){
        return 2;
    }
    Student *cur = *stu_list_ptr;
    while (cur != NULL){
        if (cur->course == course){
            ta->current_student = cur;
            cur->course->wait_time += difftime(time(NULL), *cur->arrival_time);
            cur->arrival_time += time(NULL);
            remove_student(stu_list_ptr, cur);
            return 0;
        }
        cur = cur->next_overall;
    }
    return 0;
}


/* For each course (in the same order as in the config file), print
 * the <course code>: <number of students waiting> "in queue\n" followed by
 * one line per student waiting with the format "\t%s\n" (tab name newline)
 * Uncomment and use the printf statements below. Only change the variable
 * names.
 */
void print_all_queues(Student *stu_list, Course *courses, int num_courses) {
        for (int i =0; i < num_courses; i++){
            char* course_code = courses[i].code;
            int num = get_waiting_num(courses, num_courses, &courses[i], stu_list);
            printf("%s: %d in queue\n", course_code, num);
            Student *cur = stu_list;
            while(cur != NULL){
                if (cur->course == &courses[i]){
                    printf("\t%s\n",cur->name);
                }
                cur = cur->next_overall;   
            }   
        } 
}


/*
 * Print to stdout, a list of each TA, who they are serving at from what course
 * Uncomment and use the printf statements 
 */
void print_currently_serving(Ta *ta_list) {
    if (ta_list == NULL){
        printf("No TAs are in the help centre.\n");
        return;
    }
    Ta *cur = ta_list;
    while (cur != NULL){
        if (cur->current_student != NULL){
            printf("TA: %s is serving %s from %s\n",cur->name
            , cur->current_student->name, cur->current_student->course->code);
        } else{
            printf("TA: %s has no student\n", cur->name);
        }
        cur = cur->next;
    }
    
    
    
}


/*  list all students in queue (for testing and debugging)
 *   maybe suggest it is useful for debugging but not included in marking? 
 */ 
void print_full_queue(Student *stu_list) {
    Student *cur = stu_list;
    int i = 0;
    while (cur != NULL){
        printf("%d: %s form %s", i, cur->name, cur->course->code);
        i++;
        cur = cur->next_course;
    }
    
}

/* Prints statistics to stdout for course with this course_code
 * See example output from assignment handout for formatting.
 *
 */
int stats_by_course(Student *stu_list, char *course_code, Course *courses, int num_courses, Ta *ta_list) {

    // TODO: students will complete these next pieces but not all of this 
    //       function since we want to provide the formatting
    Course *found = find_course(courses, num_courses, course_code);
    int students_waiting = find_students_waiting(found, stu_list);
    int students_being_helped = find_students_being_helped(found, ta_list);
    //Do NOT change following start;
    printf("%s:%s \n", found->code, found->description);
    printf("\t%d: waiting\n", students_waiting);
    printf("\t%d: being helped currently\n", students_being_helped);
    printf("\t%d: already helped\n", found->helped);
    printf("\t%d: gave_up\n", found->bailed);
    printf("\t%f: total time waiting\n", found->wait_time);
    printf("\t%f: total time helping\n", found->help_time);
    //Do NOT change above end.
    return 0;
}


/* Dynamically allocate space for the array course list and populate it
 * according to information in the configuration file config_filename
 * Return the number of courses in the array.
 * If the configuration file can not be opened, call perror() and exit.
 */
int config_course_list(Course **courselist_ptr, char *config_filename) {
    FILE *input_stream = fopen(config_filename, "r");
    // Read the first line
    char *this_line = (char*)malloc(sizeof(char) * INPUT_BUFFER_SIZE);
    fgets(this_line, INPUT_BUFFER_SIZE, input_stream);
    int result = strtol(this_line, NULL, 10);
    *courselist_ptr = malloc(sizeof(Course) * result);
    int i = 0;
    while (fgets(this_line, INPUT_BUFFER_SIZE, input_stream) != NULL){
        Course course;
        this_line[6] = '\0';
        strcpy(course.code, this_line);
        course.description = (char*)malloc(sizeof(char) * INPUT_BUFFER_SIZE);
        course.description = this_line + 7;
        *((*courselist_ptr) + i) = course;
        i++;
    }
    return result;
}

int find_students_waiting(Course *course, Student *stu_list){
    int result = 0;
    Student *cur = stu_list;
    while (cur != NULL){
        if (course == (cur->course)){
            result++;
        }
        cur = cur->next_overall;
    }
    return result;
}

int find_students_being_helped(Course *course, struct ta *ta_lst){
    int result = 0;
    struct ta *cur = ta_lst;
    while (cur != NULL){
        if (cur->current_student->course == course){
            result++;
        }
        cur = cur->next;
    }
    return result;
}

int is_in_queue(Student *stu_list, Student *stu){
    Student *cur = stu_list;
    while (cur != NULL){
        if (cur == stu){
            return 1;
        }
        cur = cur->next_overall;
    }
    return 0;
}

int has_course(Course *courses, char *course_code, int num_course){
    for (int i = 0; i < num_course; i++){
        if (strcmp(courses[i].code, course_code) == 0){
            return 1;
        }
    }
    return 0;
}

void remove_student(Student** student, Student* remove){
    if (*student == remove){
        *student = remove->next_overall;
        return;
    }
    Student *last = NULL;
    Student *cur = *student;
    while (cur != NULL){
        if (cur == remove){
            last->next_overall = cur->next_overall; 
            return;
        }
        last = cur;
        cur = cur->next_overall;
    }
}

int get_waiting_num(Course *courses, int num_courses, Course *course, Student* students){
    Student *cur = students;
    int result = 0;
    while (cur != NULL){
        if (cur->course == course){
            result++;
        }
        cur = cur->next_overall;
    }
    return result;
}