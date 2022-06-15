#ifndef AMN_USER_REPO_H
#define AMN_USER_REPO_H

typedef struct UserRepo UserRepo;

UserRepo* UserRepo_New(const Logger* log);
void UserRepo_Delete(UserRepo* self);



#endif // AMN_USER_REPO_H
