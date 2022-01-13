#include "server.h"
#include "dirent.h"
#include "../common.h"

bool reg(int udp_socket, char* uid, char* password, bool verbose){
    if (verbose)
        printf("UID: %s\nPassword: %s\n", uid, password);

    char path[12];
    bzero(path, 12);
    sprintf(path,"USERS/%s",uid);

    if (!(digits_only(uid, NULL) && has_correct_arg_sizes(uid, "user ID", 5, password, "password", 8) && is_alphanumerical(password, 0, false)))
        return false;

    if (!access(path, F_OK)){ // If already exists the uid directory
        udp_send(udp_socket, "RRG DUP\n", verbose);
        return true;
    }

    // Create a folder with the user uid, and a txt file with his password
    char file_path[26];
    bzero(file_path, 26);
    sprintf(file_path,"%s/%s_pass.txt", path, uid);
    if (mkdir(path, 0700) == -1){
        udp_send(udp_socket, "RRG NOK\n", verbose);
        return true;
    }

    FILE* fp = fopen(file_path, "w");
    if (!fp){
        udp_send(udp_socket, "RRG NOK\n", verbose);
        return true;
    }
    fprintf(fp,"%s",password);
    fclose(fp);

    udp_send(udp_socket, "RRG OK\n", verbose);
    return true;
}

bool unreg(int udp_socket, char* uid, char* password, bool verbose){
    if (verbose)
        printf("UID: %s\nPassword: %s\n", uid, password);

    char path[12];
    bzero(path, 12);
    sprintf(path,"USERS/%s",uid);

    if (!(digits_only(uid, NULL) && has_correct_arg_sizes(uid, "user ID", 5, password, "password", 8) && is_alphanumerical(password, 0, false)))
        return false;

    // Delete folder with the user uid, and the txt file with his password
    char file_path[26];
    bzero(file_path, 26);
    sprintf(file_path,"%s/%s_pass.txt",path,uid);
    if (access(file_path, F_OK) || remove(file_path)){
        udp_send(udp_socket, "RUN NOK\n", verbose);
        return true;
    }

    bzero(file_path, 26);
    sprintf(file_path,"%s/%s_login.txt",path,uid);
    if (!access(file_path, F_OK) && remove(file_path)){ //if the uid directory already exists
        udp_send(udp_socket, "RUN NOK\n", verbose);
        return true;
    }

    if (access(path, F_OK) == -1 || remove(path)){    //user doesnt exist
        udp_send(udp_socket, "RUN NOK\n", verbose);
        return true;
    }
    
    //Falta retirar o utilizador de todos os grupos
    DIR *d;
    bool valid;
    struct dirent *dir;
    int i = 1;
    char group_path[21], dir_name[3];
    d = opendir("GROUPS");
    if (d){
        while ((dir = readdir(d)) != NULL){
            if (dir->d_name[0]== '.' || strlen(dir->d_name) > 2)
                continue;
            bzero(group_path, 21);
            bzero(dir_name, 3);
            strcpy(dir_name, dir -> d_name);
            sprintf(group_path,"GROUPS/%s/%s.txt",dir_name,uid);
            if (access(group_path, F_OK)==0){
                if (remove(group_path)==-1){
                    udp_send(udp_socket, "RUN NOK\n", verbose);
                    return 1;
                }
            }
            if (i == 99)
                break; 
            i++;
        }
        closedir(d);
    }
    
    udp_send(udp_socket, "RUN OK\n", verbose);
    return true;
}

bool login(int udp_socket, char* uid, char* password, bool verbose){
    if (verbose)
        printf("UID: %s\nPassword: %s\n", uid, password);

    char path[12];
    bzero(path, 12);
    sprintf(path,"USERS/%s",uid);

    if (!(digits_only(uid, NULL) && has_correct_arg_sizes(uid, "user ID", 5, password, "password", 8) && is_alphanumerical(password, 0, false)))
        return false;
    
    if (access(path, F_OK) == -1){ // If the uid directory doesn't exist 
        udp_send(udp_socket, "RLO NOK\n", verbose);
        return true;
    }

    // Check if the password is correct from the uid_pass.txt
    char file_path[26], pass_temp[9];
    bzero(file_path, 26);
    sprintf(file_path,"%s/%s_pass.txt",path,uid);
    FILE *fp = fopen(file_path,"r");
    if (!fp){
        udp_send(udp_socket, "RLO NOK\n", verbose);
        return true;
    }

    fgets(pass_temp,9,fp);
    fclose(fp);

    if (strcmp(pass_temp,password)){
        udp_send(udp_socket, "RLO NOK\n", verbose);
        return true;
    }

    //create a txt file with his log
    bzero(file_path, 26);
    sprintf(file_path,"%s/%s_login.txt",path,uid);
    fp = fopen(file_path, "w");
    if (!fp){
        udp_send(udp_socket, "RLO NOK\n", verbose);
        return true;
    }
    fclose(fp);

    udp_send(udp_socket, "RLO OK\n", verbose);
    return true;
}

bool logout(int udp_socket, char* uid, char* password, bool verbose){
    if (verbose)
        printf("UID: %s\nPassword: %s\n", uid, password);

    // Check if the format is correct
    if (!(digits_only(uid, NULL) && has_correct_arg_sizes(uid, "user ID", 5, password, "password", 8) && is_alphanumerical(password, 0, false)))
        return false;
    
    char path[12];
    bzero(path, 12);
    sprintf(path,"USERS/%s",uid);

    if (access(path, F_OK) == -1){//if the uid directory exists
        udp_send(udp_socket, "ROU NOK\n", verbose);
        return true;
    }

    //Check if the password is correct from the uid_pass.txt
    char file_path[26], pass_temp[9];
    bzero(file_path, 26);
    sprintf(file_path,"%s/%s_pass.txt",path,uid);
    FILE* fp = fopen(file_path,"r");
    if (!fp){
        udp_send(udp_socket, "ROU NOK\n", verbose);
        return true;
    }
    fgets(pass_temp,9,fp);
    fclose(fp);

    if (strcmp(pass_temp,password)){
        udp_send(udp_socket, "ROU NOK\n", verbose);
        return true;
    }

    //Delete the txt file with his log
    bzero(file_path, 26);
    sprintf(file_path,"%s/%s_login.txt",path,uid);

    //Check if the login file exists
    if (access(file_path, F_OK) || remove(file_path)){ //checks if login exists and if it can delete it
        udp_send(udp_socket, "ROU NOK\n", verbose);
        return true;
    }

    udp_send(udp_socket, "ROU OK\n", verbose);
    return true;
}

void find_last_message(char* gid, char* last_msg){
    char msg_path[15];
    bzero(msg_path, 15);
    bzero(last_msg, 5);
    sprintf(msg_path,"GROUPS/%s/MSG/", gid);
    DIR* d = opendir(msg_path);
    struct dirent *dir;
    int max = 0;
    if (d)
        while ((dir = readdir(d)) != NULL)
            if (strlen(dir->d_name) == 4 && digits_only(dir->d_name, NULL) && atoi(dir->d_name) > max)
                max = atoi(dir->d_name);
    add_trailing_zeros(max, 4, last_msg);
}

int comparer(const void* x1, const void* x2){
    const group* g1 = (group*) x1;
    const group* g2 = (group*) x2;
    return strcmp(g1->gid, g2->gid);
}

int list_groups_dir(group* list, bool my_groups, char* uid){
    DIR *d;
    struct dirent *dir;
    int i = 0;
    FILE *fp;
    bool valid;
    char gidname[30],dir_name[3];
    bzero(gidname, 30);
    d = opendir("GROUPS");
    if (d){
        while ((dir = readdir(d)) != NULL){
            if (dir->d_name[0]=='.' || strlen(dir->d_name) > 2)
                continue;
            valid = true;
            bzero(dir_name, 3);
            strcpy(dir_name, dir -> d_name);
            if (my_groups){
                bzero(gidname, 30);
                sprintf(gidname,"GROUPS/%s/%s.txt",dir_name,uid);
                if (access(gidname, F_OK))
                    valid = false;
            }
            if (valid){
                bzero(gidname, 30);
                sprintf(gidname,"GROUPS/%s/%s_name.txt",dir_name,dir_name);
                fp = fopen(gidname,"r");
                if (fp){
                    strcpy(list[i].gid, dir_name);
                    fscanf(fp,"%24s",list[i].group_name);
                    fclose(fp);
                    find_last_message(dir_name, list[i].last_msg);
                    ++i;
                }   
            }
            if (i == 99)
                break;
        }
        closedir(d);
        return i;
    }
    return -1;
}

void send_groups(int udp_socket, group* list, int groups, char* message, bool verbose){
    int index;
    qsort(list, groups, sizeof(group), comparer);
    for (int i = 0; i < groups; ++i){
        index = strlen(message);
        sprintf(&(message[index]), " %s %s %s", list[i].gid, list[i].group_name, list[i].last_msg);
    }
    index = strlen(message);
    sprintf(&(message[index]), "\n");
    udp_send(udp_socket, message, verbose);
}

bool groups(int udp_socket, bool verbose){
    group list[99];
    char message[GROUPS];
    bzero(message, GROUPS);
    int groups = list_groups_dir(list, false, NULL);
    if (groups == -1)
        return false;
    sprintf(message, "RGL %d", groups);
    send_groups(udp_socket, list, groups, message, verbose);
    return true;
}

bool my_groups(int udp_socket, char* uid, bool verbose){
    if (verbose)
        printf("UID: %s\n", uid);

    group list[99];
    char message[GROUPS];
    bzero(message, GROUPS);
    if (!(is_correct_arg_size(uid, NULL, 5) && digits_only(uid, NULL)))
        return false;
    int groups = list_groups_dir(list, true, uid);
    if (groups == -1)
        return false;
    sprintf(message, "RGM %d", groups);
    send_groups(udp_socket, list, groups, message, verbose);
    return true;
}

int max_gid(){
    DIR *d;
    struct dirent *dir;
    int i = 0;
    d = opendir("GROUPS");
    if (d){
        while ((dir = readdir(d)) != NULL){
            if (dir->d_name[0]=='.' || strlen(dir->d_name) > 2)
                continue;
            if ((++i) == 99)
                break;
        }
        closedir(d);
        return i;
    }
    return -1;
}

bool subscribe(int udp_socket, char* uid, char* gid, char* group_name, bool verbose){
    bool new_group = false;

    if (verbose)
        printf("UID: %s\nGID: %s\nGroup name: %s\n", uid, gid, group_name);

    char uid_path[12];
    bzero(uid_path, 12);
    sprintf(uid_path,"USERS/%s",uid);

    //Check if the UID is well-formatted and is registered
    if (!(digits_only(uid, NULL) && is_correct_arg_size(uid, NULL, 5)) || access(uid_path, F_OK) == -1){
        udp_send(udp_socket, "RGS E_USR\n", verbose);
        return true;
    }

    int new_gid = max_gid();

    //Check if the GID is well-formatted and is registered
    if (!(digits_only(gid, NULL) && is_correct_arg_size(gid, NULL, 2)) || atoi(gid) > new_gid){
        udp_send(udp_socket, "RGS E_GRP\n", verbose);
        return true;
    }

    //Check if the group name is valid
    if (!(strlen(group_name) <= 24 && is_alphanumerical(group_name, 1, false))){
        udp_send(udp_socket, "RGS E_GNAME\n", verbose);
        return true;
    }

    //New group case
    if (!(strcmp(gid, "00"))){
        //Group database full
        if (new_gid == -1)
            return false;
        if (new_gid == 99){
            udp_send(udp_socket, "RGS E_FULL\n", verbose);
            return true;
        }
        bzero(gid, 2);
        if (++new_gid < 10)
            sprintf(gid, "0%d", new_gid);
        else
            sprintf(gid, "%d", new_gid);
        new_group = true;
    }

    char path[10], msg_path[14], name_path[22];
    bzero(path, 10);
    sprintf(path,"GROUPS/%s",gid);

    bzero(msg_path, 14);
    sprintf(msg_path,"%s/MSG", path);
 
    bzero(name_path, 22);
    sprintf(name_path,"%s/%s_name.txt", path, gid);

    if (new_group){
        if (!access(path, F_OK) || mkdir(path, 0700) == -1 || !access(msg_path, F_OK) || mkdir(msg_path, 0700) == -1){
            udp_send(udp_socket, "RGS NOK\n", verbose);
            return true;
        }
        FILE* fp = fopen(name_path, "w");
        if (!fp){
            udp_send(udp_socket, "RGS NOK\n", verbose);
            return true;
        }
        fprintf(fp, "%s", group_name);
        fclose(fp);

        bzero(name_path, 22);
        sprintf(name_path, "%s/%s.txt", path, uid);

        fp = fopen(name_path, "w");
        if (!fp){
            udp_send(udp_socket, "RGS NOK\n", verbose);
            return true;
        }
        fprintf(fp, "%s", uid);
        fclose(fp);

        char message[12];
        sprintf(message, "RGS NEW %s\n", gid);
        udp_send(udp_socket, message, verbose);
    } else {
        if (access(path, F_OK) == -1 || access(msg_path, F_OK) == -1){
            udp_send(udp_socket, "RGS NOK\n", verbose);
            return true;
        }
        FILE* fp = fopen(name_path, "r");
        if (!fp){
            udp_send(udp_socket, "RGS NOK\n", verbose);
            return true;
        }
        char group_name_temp[25];
        fgets(group_name_temp,25,fp);
        fclose(fp);

        if (strcmp(group_name_temp,group_name)){
            udp_send(udp_socket, "RGS NOK\n", verbose);
            return true;
        }

        bzero(name_path, 22);
        sprintf(name_path, "%s/%s.txt", path, uid);

        fp = fopen(name_path, "w");
        if (!fp){
            udp_send(udp_socket, "RGS NOK\n", verbose);
            return true;
        }
        fprintf(fp, "%s", uid);
        fclose(fp);

        udp_send(udp_socket, "RGS OK\n", verbose);
    }
}

bool unsubscribe(int udp_socket, char* uid, char* gid, bool verbose){

    if (verbose)
        printf("UID: %s\nGID: %s\n", uid, gid);

    char uid_path[12];
    bzero(uid_path, 12);
    sprintf(uid_path,"USERS/%s",uid);

    //Check if the UID is well-formatted
    if (!(digits_only(uid, NULL) && is_correct_arg_size(uid, NULL, 5))){
        udp_send(udp_socket, "RGU E_USR\n", verbose);
        return true;
    }

    int new_gid = max_gid();

    //Check if the GID is well-formatted and is registered
    if (!(digits_only(gid, NULL) && is_correct_arg_size(gid, NULL, 2)) || atoi(gid) > new_gid){
        udp_send(udp_socket, "RGU E_GRP\n", verbose);
        return true;
    }

    //if the group exists delete the uid file
    char file_path[22];
    bzero(file_path, 22);
    sprintf(file_path,"GROUPS/%s/%s.txt",gid,uid);

    if (!access(file_path, F_OK) && remove(file_path)){
        udp_send(udp_socket, "RGU NOK\n", verbose);
        return true;
    }

    udp_send(udp_socket, "RGU OK\n", verbose);
    return true;
}
