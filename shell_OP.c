#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <setjmp.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/dir.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>


char ipfile[100], opfile[100];

char *input;
size_t inputsize = 500;

int mflag, njobs = 0, jobnumber[1000000], ctrZ_flag, bg_flag;

sigjmp_buf ctrlz_buf;

struct stat bufferc;

int stdout1, stdin1;

char ta = 't';

typedef struct jobs
{

pid_t pid;

char name[50];

} jobs;

jobs job[1000000];


char *
whitespaces (char str[])
{

char *arr = str;

int i, j = 0;


for (i = 0; i < strlen (arr); i++)
    {

if (arr[i] == '\t')
	arr[i] = ' ';

}


for (i = 0; i < strlen (str); i++)
    {

if (str[i] == ' ')
	{

for (; str[i] == ' '; i++);

i--;

}

arr[j] = str[i];

j++;

if (arr[0] == ' ')
	j--;

}

if (arr[j - 1] == ' ')
    arr[j - 1] = '\0';

  else
    arr[j] = '\0';


return arr;

}



int
tokeniser (char input3[], char list[][30], char *t)
{

char *token;

token = strtok (input3, t);

int i = 0;


while (token)

    {

sprintf (list[i++], "%s", token);

token = strtok (NULL, t);

}

return i;

}



int
custom_pwd (char *pwd)
{

char *token;

char dir[30][30], temp[30];

sprintf (temp, "%s", pwd);

int i = 0, j, err;

for (token = strtok (temp, "/"); token != NULL;
	token = strtok (NULL, "/"), i++)

sprintf (dir[i], "%s", token);

sprintf (pwd, "/%s", dir[i - 1]);

return 0;

}



int
getPS (char mwd[])
{

char *username = (char *) malloc (30 * sizeof (char));

username = getenv ("USER");

struct utsname system_info;

uname (&system_info);

char *pwd = malloc (1000 * sizeof (char));

getcwd (pwd, 100);

if (strcmp (mwd, pwd) != 0)
    custom_pwd (pwd);

  else
    pwd[0] = '\0';

printf ("%s @ %s :~%s $ ", username, system_info.sysname, pwd);

return 0;

}



pid_t multipleCommands (char input[])
{

char list[10][30];

int status;

pid_t temp_pid, temp2_pid;


int n_args = tokeniser (input, list, ";");


mflag = 0;

if (n_args > 1)
    {

int k;

mflag = 1;

for (k = 0; k < n_args; k++)
	{

sprintf (input, "%s", list[k]);

temp_pid = fork ();

if (temp_pid == 0)

return temp_pid;

if (temp2_pid = wait (&status) < 0)
	    _exit (1);

	  else if (temp_pid > 0)
	    {

temp2_pid = wait (&status);

continue;

}

}

return -1;

}

return -2;

}



void
remove_job (pid_t pid)
{

int jno = jobnumber[pid];

int ptr;

for (ptr = jno; ptr < njobs - 1; ptr++)
    {

job[ptr] = job[ptr + 1];

jobnumber[job[ptr].pid]--;

}

njobs--;

}



int
getiofile (char input[], int in)
{

int i = 0;

if (in == 1)
    {

for (i = 0; i < strlen (input); i++)
	{

if (input[i] == '<')
	    {

for (; input[i + 1] == ' '; i++);

break;

}

}

int x = 0;

for (i++; input[i] != '>' && input[i] != ' ' && input[i] != '\n';
	    i++, x++)

ipfile[x] = input[i];

ipfile[x + 1] = '\0';

}

  else
    {

for (i = 0; i < strlen (input); i++)
	{

if (input[i] == '>')
	    {

if (input[i + 1] == '>')
		{
		  ta = 'a';
		  i++;
		}

for (; input[i + 1] == ' '; i++);

break;

}

}

int x = 0;

for (i++; input[i] != '<' && input[i] != ' ' && input[i] != '\n';
	    i++, x++)

opfile[x] = input[i];

opfile[x + 1] = '\0';

}

printf ("..in=%s..out=%s..\n", ipfile, opfile);

return 0;

}



int
outputdup (char *fileout)
{

printf ("ta=%c..\n", ta);

int fout;

if (stat (fileout, &bufferc) == 0 && access (fileout, W_OK) != 0)
    {

printf ("You dont have permission to access %s\n", fileout);

return 1;

}

  else
    {

if (stat (fileout, &bufferc))

creat (fileout, 0644);

if (ta == 't')

fout = open (fileout, O_WRONLY | O_TRUNC, 0644);

      else
	fout = open (fileout, O_WRONLY | O_APPEND, 0644);

stdout1 = dup (1);

dup2 (fout, 1);

close (fout);

}

return 0;

}



int
inputdup (char *filein)
{

int fin;

if (stat (filein, &bufferc) != 0)
    {

printf ("File %s doesn't exist\n", filein);

return 1;

}

  else if (access (filein, R_OK) != 0)
    {

printf ("You dont have permission to access %s\n", filein);

return 1;

}

  else
    {

fin = open (filein, O_RDONLY, 0644);

stdin1 = dup (0);

dup2 (fin, 0);

close (fin);

}

return 0;

}



void
iowork (char input[])
{

int k = 0;

ipfile[0] = '\0';

opfile[0] = '\0';

for (k = 0; k < strlen (input); k++)
    {

if (input[k] == '>')
	{

getiofile (input, 1);

break;

}

}

for (k = 0; k < strlen (input); k++)
    {

if (input[k] == '<')
	{

getiofile (input, 0);

break;

}

}

}


void
pipe_commands (char input[])
{

char list[10][30], parse[100], *token, ptr, temp[10][30];

int i, j, k, num = 0, fd[2], argc = 0, status, fin, fout, in = 0;

pid_t pid;


num = tokeniser (input, list, "|");


for (i = 0; i < num; i++)
    {

bg_flag = 0;


iowork (list[i]);


for (k = 0; k < strlen (list[i]); k++)
	{

if (k == '>' || k == '<' || k == '\n' || k == '\0')
	    break;

input[k] = list[i][k];

}

input[k] = '\0';


input = whitespaces (input);

sprintf (parse, "%s", input);


argc = tokeniser (parse, temp, " ");


char **argv = malloc (1000 * sizeof (char *));

for (j = 0; j < argc; j++)
	{

argv[j] = temp[j];

}


if (strcmp (temp[argc - 1], "&") == 0)
	{

bg_flag = 1;

temp[argc - 1][0] = '\0';

argc--;

}


pipe (fd);


pid = fork ();


if (pid == 0)
	{

if (fd < 0)

printf ("Pipe error\n");

	  else
	    {

if (fd[0] != 0)

		{

dup2 (in, 0);

close (fd[0]);

}

if (i != num - 1)

		{

dup2 (fd[1], 1);

close (fd[1]);

}

int ret = 0;

if (ipfile[0] != '\0')
		{

ret = 0;

ret = inputdup (ipfile);

if (ret == 1)
		    {
		      continue;
		    }

}

if (opfile[0] != '\0')
		{

ret = 0;

ret = outputdup (opfile);

if (ret == 1)
		    {
		      continue;
		    }

}


if (strcmp (temp[0], "echo") == 0)
		{

int stdout1;

if (opfile[0] != '\0')
		    {

ret = 0;

ret = outputdup (opfile);

if (ret == 1)
			{
			  continue;
			}

}

for (k = 1; k < argc; k++)

for (j = 0; j < strlen (temp[k]); j++)

printf ("%c", temp[k][j]);

printf ("\n");


dup2 (stdout1, 1);

_exit (1);

}

	      else
		{

printf ("..%s..\n", argv[0]);

execvp (argv[0], argv);

_exit (1);

}

}

}


      else if (pid > 0)
	{

if (fd < 0)

printf ("Pipe error\n");

	  else
	    {

job[njobs].pid = pid;

sprintf (job[njobs].name, "%s", temp[0]);

jobnumber[pid] = njobs;

njobs++;


if (bg_flag != 1)
		{

ctrZ_flag = 0;

waitpid (pid, &status, WUNTRACED);

if (pid < 0)
		    {

_exit (1);

}

if (pid == 0);

if (ctrZ_flag == 0)

remove_job (pid);

}

	      else

printf ("[%d] %s \n", job[njobs - 1].pid,
			 job[njobs - 1].name);

close (fd[1]);

in = fd[0];

}

}

}

}



int
pinfo_fn (char *process)
{

int i = 0;

FILE * fd;

char add[100];

char text[100], exe[100], state, size[10], extra_arr[100] = "a";

for (i = 0; i < 100; i++)

add[i] = '\0';

add[0] = '/';
  add[1] = 'p';
  add[2] = 'r';
  add[3] = 'o';
  add[4] = 'c';
  add[5] = '/';
  add[6] = '\0';

if (process[0] == '\0')
    {

sprintf (process, "%d", getpid ());

}

printf ("Pid -- %s\n", process);

for (i = 0; process[i] != '\0'; i++)

add[6 + i] = process[i];

add[6 + i] = '\0';

strcpy (exe, add);

for (i = 0; add[i] != '\0'; i++);

exe[i] = '/';
  exe[i + 1] = 'e';
  exe[i + 2] = 'x';
  exe[i + 3] = 'e';
  exe[i + 4] = '\0';

for (i = 0; add[i] != '\0'; i++);

add[i] = '/';
  add[i + 1] = 's';
  add[i + 2] = 't';
  add[i + 3] = 'a';
  add[i + 4] = 't';
  add[i + 5] == '\0';

fd = fopen (add, "r");

if (fd == NULL)
    {

printf ("Process does not exist.\n");
      return 0;

}

fscanf (fd, "%s", text);

fscanf (fd, "%s", text);

fscanf (fd, "%s", text);

printf ("Process Status -- %s\n", text);

for (i = 0; add[i] != '\0'; i++);

add[i] = 'm';
  add[i + 1] = '\0';

fd = fopen (add, "r");

if (fd == NULL)
    {

printf ("Process does not exist.\n");
      return 0;

}

fscanf (fd, "%s", text);

printf ("Memory -- %s\n", text);

printf ("Executable -- ");

int ret = readlink (exe, extra_arr, 100);

printf ("%s\n", extra_arr);

return 0;

}




void
signalHandlerc (int sig_num)
{

signal (SIGINT, signalHandlerc);

printf ("ctrl c pressed \n");

fflush (stdout);

}
void

signalHandlerz (int sig_num)
{

signal (SIGTSTP, signalHandlerz);

printf ("ctrl z pressed \n");

fflush (stdout);

}

int

run_fn ()
{

pid_t Mpid = getpid ();

char *input = malloc (1000 * sizeof (char));

char *input2 = malloc (1000 * sizeof (char));

char mwd[100], pwd[100], p[10][30];

pid_t pid, cpid, err;

int i, len, j, status, k, a, argc, fin, fout;


getcwd (mwd, 100);


signal (SIGINT, signalHandlerc);

signal (SIGTSTP, signalHandlerz);


while (1)
    {

getPS (mwd);


input[0] = '\0';

ipfile[0] = '\0';

opfile[0] = '\0';

ta = 't';


getline (&input, &inputsize, stdin);

input[strlen (input) - 1] = '\0';


sprintf (input2, "%s", input);

pid_t mpid = multipleCommands (input);

if (mpid == -1)
	continue;

int piper = 0, pipflag = 0;

for (i = 0; i < strlen (input); i++)
	{

if (pipflag == 1)
	    {
	      break;
	    }

if (input[i] == '|')
	    {

piper = 1;

pipe_commands (input);

pipflag = 1;

}

}

if (piper == 1 && mpid == 0 && mflag == 1)

exit (10);


      else if (piper == 1 && mflag == 0)

continue;


ipfile[0] = '\0';

opfile[0] = '\0';

for (i = 0; i < strlen (input); i++)
	{

if (input[i] == '<')
	    getiofile (input, 1);

	  else if (input[i] == '>')
	    getiofile (input, 0);

}


for (int ite = 0;
	      ite < strlen (input) && input[ite] != '<' && input[ite] != '>';
	      ite++)

input2[ite] = input[ite];

input2 = whitespaces (input2);

sprintf (input, "%s", input2);

if (strcmp (input, "exit") == 0 || strcmp (input, "quit") == 0)
	{

printf ("Exiting shell BYE BYE\n\n");

kill (Mpid, 9);

}


if (input[0] == '\0' || input[0] == '\n')
	{

printf ("\n");

continue;

}


if (strcmp (input, "pwd") == 0)
	{

if (opfile[0] != '\0')
	    {

if (outputdup (opfile) == 1)
		continue;

}

getcwd (pwd, 100);

printf ("%s\n", pwd);

dup2 (stdout1, 1);

}


      else
	{

bg_flag = 0;

if (input[strlen (input) - 1] == '&')
	    {

bg_flag = 1;

input[strlen (input) - 2] = '\0';

}

argc = tokeniser (input, p, " ");


if (strcmp (p[0], "pinfo") == 0)
	    {

pinfo_fn (p[1]);

}

if (strcmp (p[0], "cd") == 0)
	    {

if (argc == 1)

chdir (mwd);

	      else if (argc > 1)
		{

if (p[1][0] == '~' && p[1][1] == '\0')
		    {
		      chdir (mwd);
		    }

		  else
		    {

DIR * dir = opendir (p[1]);

if (dir)
			{
			  closedir (dir);
			  chdir (p[1]);
			}

		      else
			printf ("No such directory exists\n");

}

}

	      else

printf ("No such directory exists\n");

getcwd (pwd, 100);

}


	  else if (strcmp (p[0], "echo") == 0)
	    {

if (opfile[0] != '\0')
		{

if (outputdup (opfile) == 1)
		    continue;

}

for (k = 1; k < argc && p[k][0] != '>' && p[k][0] != '<'; k++)
		{

for (j = 0; j < strlen (p[k]); j++)

printf ("%c", p[k][j]);

printf (" ");

}

printf ("\n");

dup2 (stdout1, 1);

}


	  else if (strcmp (p[0], "jobs") == 0)
	    {

if (njobs == 0)

printf ("No jobs to show\n");

if (njobs > 0)
		{

for (i = 0; i < njobs; i++)
		    {

char addr[100] = "/proc/";

char arrtemp[100];

sprintf (arrtemp, "%d", job[i].pid);

strcat (addr, arrtemp);

strcat (addr, "/stat");

char text[100];

FILE * fd;

fd = fopen (addr, "r");

fscanf (fd, "%s", text);

fscanf (fd, "%s", text);

fscanf (fd, "%s", text);

if (text[0] == 'R')
			{
			  sprintf (text, "Running");
			}

		      else if (text[0] == 'S')
			{
			  sprintf (text, "Stopped");
			}

printf ("[%d] %s %s [%d]\n", i + 1, job[i].name, text,
			       job[i].pid);

free (fd);

}

}

}


	  else if (strcmp (p[0], "kjob") == 0)
	    {

if (argc == 3)
		{

int sig = atoi (p[2]);

int j_no = atoi (p[1]) - 1;

if (j_no < njobs)
		    {

kill (job[j_no].pid, sig);

remove_job (job[j_no].pid);

}

		  else
		    printf ("No such job exists such as %d\n", j_no);

}

	      else
		printf ("Wrong usage of kjob\n");

}


	  else if (strcmp (p[0], "overkill") == 0)
	    {

stdout1 = dup (1);

if (opfile[0] != '\0')
		{

if (outputdup (opfile) == 1)
		    continue;

}

if (njobs == 0)
		printf ("No jobs to kill\n");

	      else
		{

for (i = 0; i < njobs; i++)
		    {

printf ("[%d] %s [%d] killed \n", i + 1, job[i].name,
			       job[i].pid);

kill (job[i].pid, 9);

remove_job (job[i].pid);

}

sleep (10);

}

dup2 (stdout1, 1);

printf ("Done\n");

}


	  else if (strcmp (p[0], "fg") == 0)
	    {

if (argc == 2)
		{

int j_no = atoi (p[1]) - 1;

if (j_no < njobs)
		    {

int status;

ctrZ_flag = 0;

kill (job[atoi (p[1]) - 1].pid, SIGCONT);

pid_t pid =
			waitpid (job[atoi (p[1]) - 1].pid, &status,
				 WUNTRACED);


if (ctrZ_flag == 1)
			{

printf ("%s with pid:%d killed\n", job[j_no].name,
				   pid);

remove_job (job[i].pid);

sleep (0);

}

}

		  else
		    printf ("No such job exists\n");

}

	      else
		printf ("Wrong usage of fg\n");

}


	  else if (strcmp (p[0], "bg") == 0)
	    {

if (argc == 2)
		{

int j_no = atoi (p[1]) - 1;

if (j_no < njobs)
		    {

kill (job[atoi (p[1]) - 1].pid, SIGCONT);

}

}

	      else
		printf ("No shuch job exists\n");

}


	  else if (strcmp (p[0], "setenv") == 0)
	    {

if (argc == 3)
		{

setenv (p[1], p[2], 1);

printf ("Environment variable set:: %s : %s\n", p[1],
			   p[2]);

}

	      else
		printf ("Wrong usage of setenv\n");

}


	  else if (strcmp (p[0], "unsetenv") == 0)
	    {

if (argc == 2)
		{

unsetenv (p[1]);

printf ("Environment variable unset:: %s\n", p[1]);

}

	      else
		printf ("Wrong usage of unsetenv\n");

}

	  else
	    {

char **argv = malloc (265 * sizeof (char *));

for (j = 0; j < argc && p[j][0] != '<' && p[j][0] != '>'; j++)
		{
		  argv[j] = p[j];
		}

pid = fork ();

if (pid > 0)
		{

job[njobs].pid = pid;

sprintf (job[njobs].name, "%s", p[0]);

jobnumber[pid] = njobs;

njobs++;


if (bg_flag != 1)
		    {

ctrZ_flag = 0;

waitpid (pid, &status, WUNTRACED);

if (pid < 0)
			{

perror ("Error!");

_exit (1);

}

if (ctrZ_flag == 0)
			{

remove_job (pid);

kill (pid, 9);

}

		      else if (ctrZ_flag == 1);

}

		  else

printf ("[%d] %s \n", job[njobs - 1].pid,
			     job[njobs - 1].name);

}

	      else if (pid == 0)
		{

if (ipfile[0] != '\0')
		    {

if (inputdup (ipfile) == 1)
			continue;

}

if (opfile[0] != '\0')
		    {

if (outputdup (opfile) == 1)
			continue;

}


execvp (argv[0], argv);

dup2 (stdin1, 0);

dup2 (stdout1, 1);

_exit (1);

}

}

}

if (mpid == 0 && mflag == 1)
	{

exit (16);

}

}

return 0;

}



int
main ()
{

run_fn ();

return 0;

}
