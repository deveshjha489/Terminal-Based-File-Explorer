#include "headers.h"
int pos = 1;
using namespace std;
struct termios curr_config;
vector<dirent *> dir_name;
stack<string> bck, forw;
char home[PATH_SIZE];
int dirCount, rows;
string get_time(time_t unix_time) {
  struct tm *timeinfo;
  timeinfo = localtime(&unix_time);
  string mod_stamp = asctime(timeinfo), mod_time = "";
  for (int i = 4; i < 16; i++) {
    mod_time += mod_stamp[i];
  }
  return mod_time;
}
bool isDir(string pathstr){
  struct stat buf;
  DIR *dir;
  char path[PATH_SIZE];
  strcpy(path, pathstr.c_str());
  dir = opendir(path);
  bool ret = true;
  if (dir == NULL){
    ret = false;
  }
  closedir(dir);
  return ret;
}
void printList(vector<string> v) {
  int len = v.size();
  for (int i = 0; i < len; i++) {
  	if(i == 1)cout<<"\t";
    cout << setw(13) << left << v[i];
  }
  cout << endl;
}
void resetTerminal() {
  tcsetattr(0, TCSANOW, &curr_config);
  clrscr();
}
void setTerminal() {
  tcgetattr(0, &curr_config);
  struct termios new_config = curr_config;
  new_config.c_lflag &= ~(ICANON);  // disable canonical mode
  new_config.c_lflag &= ~(ECHO);    // disable echo of character
  tcsetattr(0, TCSANOW, &new_config);
}
void ls() {
  char curr_dir[PATH_SIZE];
  struct dirent *dir_str;
  struct stat buf;
  struct passwd *user_info;
  struct group *grp_info;
  DIR *dir_ptr;
  string pwd = getcwd(
      curr_dir,
      PATH_SIZE);  // get current working directory // clog << curr_dir << endl;
  dir_ptr = opendir(curr_dir);
  vector<string> v;
  dir_name.clear();
  if (dir_ptr == NULL) {
    cout << "Error opening directory\n";
  } else {
    while ((dir_str = readdir(dir_ptr)) != NULL) {
      ++dirCount;
      int status = stat(dir_str->d_name, &buf);
      //cout<<dir_str->d_name;
      if (status == -1) {
        cout << "stat error";
        break;
      }
      string permission = "";
      if (S_ISDIR(buf.st_mode))  // check if directory
        permission += "d";       // cout << "d";
      else
        permission += "-";  // cout << "-";
      if (buf.st_mode & S_IRUSR)
        permission += "r";  // cout << "r";
      else
        permission += "-";  // cout << "-";
      if (buf.st_mode & S_IWUSR)
        permission += "w";  // cout << "w";
      else
        permission += "-";  // cout << "-";
      if (buf.st_mode & S_IXUSR)
        permission += "x";  // cout << "x";write(1,"\033[1A",4)
      else
        permission += "-";  // cout << "-";
      if (buf.st_mode & S_IRGRP)
        permission += "r";  // cout << "r";
      else
        permission += "-";  // cout << "-";
      if (buf.st_mode & S_IWGRP)
        permission += "w";  // cout << "w";
      else
        permission += "-";  // cout << "-";
      if (buf.st_mode & S_IXGRP)
        permission += "x";  // cout << "x";
      else
        permission += "-";  // cout << "-";
      if (buf.st_mode & S_IROTH)
        permission += "r";  // cout << "r";
      else
        permission += "-";  // cout << "-";
      if (buf.st_mode & S_IWOTH)
        permission += "w";  // cout << "w";
      else
        permission += "-";  // cout << "-";
      if (buf.st_mode & S_IXGRP)
        permission += "x";  // cout << "x";
      else
        permission += "-";  // cout << "-";

      user_info = getpwuid(buf.st_uid);
      grp_info = getgrgid(buf.st_gid);
      v.push_back(dir_str->d_name);
      v.push_back(permission);
      v.push_back(string(user_info->pw_name));
      v.push_back((string)grp_info->gr_name);
      v.push_back(to_string(buf.st_size));
      v.push_back(get_time(buf.st_mtime));
      dir_name.push_back(dir_str);
      printList(v);
      v.clear();
    }
  }
  closedir(dir_ptr);
}
void resetLS() {
  clrscr();
  poscur();
  ls();
  poscur();
  pos = 1;
}
void open(int row) {
  struct stat buf;
  struct dirent *dir_str = dir_name[row - 1];
  stat(dir_str->d_name, &buf);
  if (S_ISDIR(buf.st_mode)) {
    char dir_loc[PATH_SIZE];
    string path = getcwd(dir_loc, PATH_SIZE);
    bck.push(path);
    while(!forw.empty()){
      forw.pop();
    }
    string app = dir_str->d_name;
    path += "/" + app;
    int len = path.length();
    char path_arr[len + 1];
    strcpy(path_arr, path.c_str());
    dirCount = 0;
    chdir(path_arr);
    resetLS();
  } else {
    char *filename = dir_str->d_name;
    pid_t pid = fork();
    if (pid == 0) {
      execl("/usr/bin/vi", "vi", filename, NULL);
      exit(0);
    }
    wait(NULL);
  }
  pos = 1;
}
// When left arrow is pressed
void goBack() {
  if (bck.size() == 0) {
    return;
  }
  string path = bck.top();
  int len = path.length();
  // printf("\033[%d;1H", rows);
  char path_arr[len + 1], tmp[PATH_SIZE];
  strcpy(path_arr, path.c_str());
  string new_path = getcwd(tmp, PATH_SIZE);
  forw.push(new_path);
  if (!bck.empty()) {
    dirCount = 0;
    chdir(path_arr);
    bck.pop();
    resetLS();
  }
}
// When right arrow is pressed
void goForward() {
  if (forw.size() == 0) {
    return;
  }
  string path = forw.top();
  int len = path.length();
  char path_arr[len + 1], tmp[PATH_SIZE];
  strcpy(path_arr, path.c_str());
  string new_path = getcwd(tmp, PATH_SIZE);
  bck.push(new_path);
  if (!forw.empty()) {
    dirCount = 0;
    chdir(path_arr);
    forw.pop();
    resetLS();
  }
}
// When Backspace is Pressed
void goToParent() {
  char path_arr[PATH_SIZE];
  string temp = getcwd(path_arr, PATH_SIZE);
  bck.push(temp);
  int len = temp.length(), i = len - 1;
  for (; temp[i] != '/'; i--)
    ;
  string ntemp = temp.substr(0, i);
  char parent[ntemp.length() + 1];
  strcpy(parent, ntemp.c_str());
  dirCount = 0;
  chdir(parent);
  resetLS();
}
vector<string> getCommand(string cmdString) {
  int i = 0, len = cmdString.length();
  vector<string> arguments;
  string cmdName = "";
  while (i < len) {
    if (cmdString[i] == ' ') {
      arguments.push_back(cmdName);
      cmdName = "";
    } else
      cmdName += cmdString[i];
    i++;
  }
  arguments.push_back(cmdName);
  return arguments;
}
string checkAbsolute(string path) {
  if (path[0] == '~') {
    return path.substr(2, path.length());
  }else return path;
}
void createDir(string filename, string dest) {
	dest = checkAbsolute(dest);
	char new_dir[PATH_SIZE];
	if(dest[0] == '.'){
		getcwd(new_dir,PATH_SIZE);
		dest = string(new_dir);
	}else if(dest[0] != '/'){
		dest = string(home) + "/" + dest;
	}
  string dirPath = dest + "/" + filename;
  strcpy(new_dir, dirPath.c_str());
  int status = mkdir(new_dir, 0777);  // 0777 all permissions
  if (status == 0) {
    //
  } else {
    // error;
  }
  printf("\33[2K");  // clears the line on curr cursor pos
  printf("\033[%d;1H", rows);
  cout << "dir created successfully";
}
void cd(string destination) {
	destination = checkAbsolute(destination);
  char dest[PATH_SIZE], src[PATH_SIZE];
  if (destination[0] == '.') { 
    getcwd(dest,PATH_SIZE);
    destination = string(dest);
  }else if(destination[0] != '/'){
     destination = string(home) + "/" + destination; 	
  }
  strcpy(dest, destination.c_str());
  string curr = getcwd(src, PATH_SIZE);
  chdir(dest);
  bck.push(curr);
  dirCount = 0;
  resetLS();
  printf("\033[%d;1H", rows);
}
void removeFile(string filepath) {
	filepath = checkAbsolute(filepath);
	char path[PATH_SIZE];
  if (filepath[0] == '.') { 
    getcwd(path,PATH_SIZE);
    filepath = string(path);
  }else if(filepath[0] != '/'){
  	filepath = string(home) + "/" + filepath;
  }
  strcpy(path, filepath.c_str());
  int status = remove(path);
  printf("\33[2K");  // clears the line on curr cursor pos
  printf("\033[%d;1H", rows);
}
void removeDir(string dirPath){
	char path[PATH_SIZE];
  strcpy(path, dirPath.c_str());
  int status = rmdir(path);
}
void dfs(string root) {
  struct dirent *dt;
  struct stat buf;
  DIR *dir;
  char path[PATH_SIZE];
  strcpy(path, root.c_str());
  dir = opendir(path);
  if (dir == NULL){
    if(errno == ENOTDIR){ //if dir is NULL and errno is set to ENOTDIR then it is a file else invalid dir
    	//delete file
     removeFile(root);
     //cout<<"Deleting "<<root<<endl;
    }
    return;
  }
  while ((dt = readdir(dir)) != NULL) {
    string dname = dt->d_name;
    bool iscurr = false, isprev = false;
    if (dname == ".") iscurr = true;
    if (dname == "..") isprev = true;
    if (!iscurr && !isprev) {
      dname = root + "/" + dname;
      dfs(dname);          
    }
  }
  //Delete Dir
    //cout<<"Deleting "<<root<<endl;
  removeDir(root);
}
void removeDirUtil(string dirPath){
	dirPath = checkAbsolute(dirPath);
	char dest[PATH_SIZE];
  if (dirPath[0] == '.') { 
    getcwd(dest,PATH_SIZE);
    dirPath = string(dest);
  }else{
     dirPath = string(home) + "/" + dirPath; 	
  }
  dfs(dirPath);
}
void renameFile(string oldFile, string newFile) {
  char oldFilePath[PATH_SIZE], newFilePath[PATH_SIZE];
  oldFile = checkAbsolute(oldFile);
  newFile = checkAbsolute(newFile);
  if (oldFile[0] != '/') {
    oldFile = string(home) + "/" + oldFile;
    newFile = string(home) + "/" + newFile;
  }
  strcpy(oldFilePath, oldFile.c_str());
  strcpy(newFilePath, newFile.c_str());
  int status = rename(oldFilePath, newFilePath);
  printf("\33[2K");  // clears the line on curr cursor pos
  dirCount = 0;
  resetLS();
  printf("\033[%d;1H", rows);
}
void createFile(string fileName, string destination) {
  destination = checkAbsolute(destination);
  /*
      .
      ~/Parent -> /home/parent
      /home/Parent ->no change
  */
  char path[PATH_SIZE];
  if (destination[0] == '.') { 
    getcwd(path,PATH_SIZE);
    destination = string(path);
  }else if(destination[0] != '/'){
  	destination = string(home) + "/" + destination;
  }
  string file = destination + "/" + fileName;
  strcpy(path, file.c_str());
  creat(path, 0777);  // All permission
  printf("\33[2K");   // clears the line on curr cursor pos
  dirCount = 0;
  resetLS();
  printf("\033[%d;1H", rows);
}
bool bfsSearch(string key){
  queue<string> q;
  q.push(string(home));
  while(!q.empty()){
  	 string root = q.front();
  	 q.pop();
  	 struct dirent *dt;
	 	 struct stat buf;
	 	 DIR *dir;
	 	 bool isDir = true;
	 	 char path[PATH_SIZE];
	 	 strcpy(path,root.c_str());
	 	 dir = opendir(path);
	 	 if(dir == NULL){
	 	 	continue;
	 	 }
     while ((dt = readdir(dir)) != NULL){
     	 string dname = dt->d_name;
     	 bool iscurr = false , isprev = false;
     	 if(dname == ".")iscurr = true;
     	 if(dname == "..")isprev = true;
     	 if(dname == key)return true;
     	 if(!iscurr && !isprev){
     	 	 dname = root + "/" + dname;  
  			 q.push(dname);	 
     	 }
     }
     closedir(dir);
  }
  return false;
}
int getPermission(char* curr_dir){
  struct stat buf;
  int status = stat(curr_dir, &buf);
  int permission = 0;
  if(buf.st_mode & S_IRUSR){
      permission |= (buf.st_mode & S_IRUSR);
  }
  if(buf.st_mode & S_IWUSR){
    permission |= (buf.st_mode & S_IWUSR);
  }
  if(buf.st_mode & S_IXUSR){
    permission |= (buf.st_mode & S_IXUSR);
  }
  if(buf.st_mode & S_IROTH){
    permission |= (buf.st_mode & S_IROTH);
  }
  if(buf.st_mode & S_IWOTH){
    permission |= (buf.st_mode & S_IWOTH);
  }
  if(buf.st_mode & S_IXOTH){
    permission |= (buf.st_mode & S_IXOTH);
  }
  if(buf.st_mode & S_IRGRP){
    permission |= (buf.st_mode & S_IRGRP);
  }
  if(buf.st_mode & S_IWGRP){
    permission |= (buf.st_mode & S_IWGRP);
  }
  if(buf.st_mode & S_IXGRP){
    permission |= (buf.st_mode & S_IXGRP);
  }
  return permission;
}
void copyFile(string filename,string src,string dest){
   char filechar[PATH_SIZE];
   strcpy(filechar,filename.c_str());
   int permission = getPermission(filechar);
  // clog<<"permission "<<permission<<endl;
   char fileBuffer[BUFFER];
   size_t fileSize;
   dest = dest + '/' + filename;
   char srcpath[PATH_SIZE],destpath[PATH_SIZE];
  // clog<<"copying srcpath\n";
   strcpy(srcpath,src.c_str());
   //clog<<"copying destpath\n";
   strcpy(destpath,dest.c_str());
   //cout<<"copying from : "<<srcpath<<" to "<<destpath<<endl;
   FILE *source = fopen(srcpath,"rb");
   FILE *destination = fopen(destpath,"wb");

   while(fileSize = fread(fileBuffer,1,BUFFER,source)){
      fwrite(fileBuffer, 1, fileSize, destination);
   }
   chmod(destpath,permission);
   fclose(source);
   fclose(destination);
}
string getPath(string pathstr){
  pathstr = checkAbsolute(pathstr);
	char path[PATH_SIZE];
	if(pathstr[0] == '.'){
		getcwd(path,PATH_SIZE);
		pathstr = string(path);
	}else if(pathstr[0] != '/'){
		pathstr = string(home) + "/" + pathstr;
	}
  return pathstr;
}
void copyUtil(vector<string> attr){
  vector<string> srcname;
  int len = attr.size();
  for(int i=1;i<len-1;i++){
    srcname.push_back(attr[i]);
  }
  string dest = getPath(attr[len-1]);
  for(auto i : srcname){
    string src = getPath(i);
    //clog<<"INSIDE COPY_UTIL : "<<src<<endl;
    if(isDir(src)){
     // copyDir(i,src,dest);
    }else{
      copyFile(i,src,dest);
      printf("\33[2K");   // clears the line on curr cursor pos
      printf("\033[%d;1H", rows);
    }
  }
}
void performOperation(string cmdString) {
  vector<string> attr = getCommand(cmdString);
  string cmdName = attr[0];
  if (cmdName == "copy") {
  	if(attr.size() >= 3){
  		copyUtil(attr);
  	}else{
  		//error
  	}
  } else if (cmdName == "move") {
  } else if (cmdName == "rename") {
    if (attr.size() == 3) {
      renameFile(attr[1], attr[2]);
    } else {
      // error
    }
  } else if (cmdName == "create_file") {
    if (attr.size() == 3) {
      createFile(attr[1], attr[2]);
    } else {
      // error
    }
  } else if (cmdName == "create_dir") {
    if (attr.size() == 3) {
      createDir(attr[1], attr[2]);
    } else {
      // error
    }
  } else if (cmdName == "delete_file") {
    if (attr.size() == 2) {
      removeFile(attr[1]);
    } else {
      // error
    }
  } else if (cmdName == "delete_dir") {
  	removeDirUtil(attr[1]);
  	resetLS();
  	dirCount = 0;
  } else if (cmdName == "goto") {
    if (attr.size() == 2) {
      cd(attr[1]);
    } else {
      // error
    }
  } else if (cmdName == "search") {
  	  printf("\33[2K");   // clears the line on curr cursor pos
      printf("\033[%d;1H", rows);
  	if(bfsSearch(attr[1])){
  		cout<<"True";
  	}else cout<<"False";
  }
}
void commandmode() {
  struct winsize size;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
  rows = size.ws_row;
  printf("\033[%d;1H", rows);  // move to bottom left
  string res = "";
  char key;
  int len = 0;
  while (1) {
    key = cin.get();
    if (key == ENTER) {
      // cout<<"Enter key pressed";
      performOperation(res);
    } else if (key == ESCAPE) {
      dirCount = 0;
      resetLS();
      break;
    } else if (key == BACKSPACE) {
      len--;
      printf("\33[2K");            // clears the line on curr cursor pos
      printf("\033[%d;1H", rows);  // moves the cursor to extreme left
      res = res.substr(0, len);
      cout << res;
    } else {
      len++;
      res = res + key;
      cout << key;
    }
  }
}
int main() {
  clrscr();
  setTerminal();
  getcwd(home, PATH_SIZE);
  ls();
  poscur();
  pos = 1;
  char key;
  while (true) {
    key = cin.get();
    if (key == UP) {
      if (pos > 1) {
        mvup();
        pos--;
      }
    } else if (key == DOWN) {
      if (pos < dirCount) {
        mvdwn();
        pos++;
      }
    } else if (key == LEFT) {
      // cout<<"pressed left key";
      goBack();
    } else if (key == RIGHT) {
      goForward();
    } else if (key == ENTER) {
      open(pos);
    } else if (key == BACKSPACE) {
      goToParent();
    } else if (key == 'h') {
      char cr[PATH_SIZE];
      string curr = getcwd(cr, PATH_SIZE);
      bck.push(curr);
      dirCount = 0;
      chdir(home);
      resetLS();
    } else if (key == ':') {
      commandmode();
      //  cout<<"Command Mode Exited\n";
    } else if (key == QUIT)
      break;
  }
  resetTerminal();
  return 0;
}
/*
1.)[DONE] Handle Enter on . and ..
2.) Handle Enter on file
3) [DONE]Restrict cursor movement after last line
4.) Handle prev/forw for enter on . and ..
5.) create_file dest path ..relative handle
*/
