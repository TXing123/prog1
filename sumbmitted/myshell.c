/* 
 * Skeleton code for Shell processing
 * This file contains skeleton code for executing commands parsed in main-x.c.
 * Acknowledgement: derived from UCLA CS111
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "cmdline.h"
#include "myshell.h"

/** 
 * Reports the creation of a background job in the following format:
 *  [job_id] process_id
 * to stderr.
 */
int jobID=1;
void report_background_job(int job_id, int process_id);

/* command_exec(cmd, pass_pipefd)
 *
 *   Execute the single command specified in the 'cmd' command structure.
 *
 *   The 'pass_pipefd' argument is used for pipes.
 *   On input, '*pass_pipefd' is the file descriptor used to read the
 *   previous command's output.  That is, it's the read end of the previous
 *   pipe.  It equals STDIN_FILENO if there was no previous pipe.
 *   On output, command_exec should set '*pass_pipefd' to the file descriptor
 *   used for reading from THIS command's pipe.
 *   If this command didn't have a pipe -- that is, if cmd->commandop != PIPE
 *   -- then it should set '*pass_pipefd = STDIN_FILENO'.
 *
 *   Returns the process ID of the forked child, or < 0 if some system call
 *   fails.
 *
 *   You must also handle the internal commands "cd" and "exit".
 *   These are special because they must execute in the shell process, rather
 *   than a child.  (Why?)
 *
 *   However, these special commands still have a status!
 *   For example, "cd DIR" should return status 0 if we successfully change
 *   to the DIR directory, and status 1 otherwise.
 *   Thus, "cd /tmp && echo /tmp exists" should print "/tmp exists" to stdout
 *   iff the /tmp directory exists.
 *   Not only this, but redirections should work too!
 *   For example, "cd /tmp > foo" should create an empty file named 'foo';
 *   and "cd /tmp 2> foo" should print any error messages to 'foo'.
 *
 *   How can you return a status, and do redirections, for a command executed
 *   in the parent shell?
 *   Hint: It is easiest if you fork a child ANYWAY!
 *   You should divide functionality between the parent and the child.
 *   Some functions will be executed in each process.
 */
static pid_t
command_exec(command_t *cmd, int *pass_pipefd)
{
	pid_t pid = -1;		// process ID for child
	int pipefd[2];		// file descriptors for this process's pipe
	int pipe_in=*pass_pipefd;
	/* EXERCISE: Complete this function!
	 * We've written some of the skeleton for you, but feel free to
	 * change it.
	 */

	// Create a pipe, if this command is the left-hand side of a pipe.
	// Return -1 if the pipe fails.
	if (cmd->controlop == CMD_PIPE) {
		/* Your code here. */
		if(pipe(pipefd)<0){
			perror("pipe error");
			abort();
		}
	}
	else{
		*pass_pipefd=STDIN_FILENO;
	}


	// Fork the child and execute the command in that child.
	// You will handle all redirections by manipulating file descriptors.
	//
	// This section is fairly long.  It is probably best to implement this
	// part in stages, checking it after each step.  For instance, first
	// implement just the fork and the execute in the child.  This should
	// allow you to execute simple commands like 'ls'.  Then add support
	// for redirections: commands like 'ls > foo' and 'cat < foo'.  Then
	// add parentheses, then pipes, and finally the internal commands
	// 'cd' and 'exit'.
	//
	// In the child, you should:
	//    1. Set up stdout to point to this command's pipe, if necessary.
	//    2. Set up stdin to point to the PREVIOUS command's pipe (that
	//       is, *pass_pipefd), if appropriate.
	//    3. Close some file descriptors.  Hint: Consider the read end
	//       of this process's pipe.
	//    4. Set up redirections.
	//       Hint: For output redirections (stdout and stderr), the 'mode'
	//       argument of open() should be set to 0666.
	//    5. Execute the command.
	//       There are some special cases:
	//       a. Parentheses.  Execute cmd->subshell.  (How?)
	//       b. A null command (no subshell, no arguments).
	//          Exit with status 0.
	//       c. "exit".
	//       d. "cd".
	//
	// In the parent, you should:
	//    1. Close some file descriptors.  Hint: Consider the write end
	//       of this command's pipe, and one other fd as well.
	//    2. Handle the special "exit" and "cd" commands.
	//    3. Set *pass_pipefd as appropriate.
	//
	// "cd" error note:
	// 	- Upon syntax errors: Display the message
	//	  "cd: Syntax error on bad number of arguments"
	// 	- Upon system call errors: Call perror("cd")
	//
	// "cd" Hints:
	//    For the "cd" command, you should change directories AFTER
	//    the fork(), not before it.  Why?
	//    Design some tests with 'bash' that will tell you the answer.
	//    For example, try "cd /tmp ; cd $HOME > foo".  In which directory
	//    does foo appear, /tmp or $HOME?  If you chdir() BEFORE the fork,
	//    in which directory would foo appear, /tmp or $HOME?
	//

	/* Your code here. */
/*
O_RDONLY	Open the file so that it is read only.
O_WRONLY	Open the file so that it is write only.
O_RDWR	Open the file so that it can be read from and written to.
O_APPEND	Append new information to the end of the file.
O_TRUNC	Initially clear all data from the file.
O_CREAT	If the file does not exist, create it. If the O_CREAT option is used, then you must include the third parameter.
O_EXCL	Combined with the O_CREAT option, it ensures that the caller must create the file. If the file already exists, the call will fail.
*/
	pid=fork();
	int FileDis;
	if(pid==-1){
		perror("fork error\n");
		abort();
	}
	else if(pid==0){//child
		if(cmd->controlop==CMD_PIPE){
			close(pipefd[0]);
			if(dup2(pipefd[1],STDOUT_FILENO)<0){
				perror("PIPE write redirection fail");
				abort();
			}
			close(pipefd[1]);
		}
		if(cmd->redirect_filename[0]!=NULL){
			FileDis=open(cmd->redirect_filename[0],O_RDONLY);
			if(FileDis<0){
				perror("input redirections: open fail");
				abort();
			}
			if(dup2(FileDis,STDIN_FILENO)<0){
				perror("input redirections: redirect fail");
				abort();
			}
			close(FileDis);
		}
		else if(pipe_in!=STDIN_FILENO){
			if(dup2(pipe_in,STDIN_FILENO)<0){
				perror("PIPE read redirection fail");
				abort();
			}
			close(pipe_in);
		}
		if(cmd->redirect_filename[1]!=NULL){
			FileDis=open(cmd->redirect_filename[1],O_CREAT|O_WRONLY|O_TRUNC,S_IRWXU|S_IRWXG|S_IRWXO);
			if(FileDis<0){
				perror("output redirections: open fail");
				abort();
			}
			if(dup2(FileDis,STDOUT_FILENO)<0){
				perror("output redirections: redirect fail");
				abort();
			}
			close(FileDis);
		}
		if(cmd->redirect_filename[2]!=NULL){
			FileDis=open(cmd->redirect_filename[2],O_CREAT|O_WRONLY|O_TRUNC,S_IRWXU|S_IRWXG|S_IRWXO);
			if(FileDis<0){
				perror("error redirections: open fail");
				abort();
			}
			if(dup2(FileDis,STDERR_FILENO)<0){
				perror("error redirections: redirect fail");
				abort();
			}
			close(FileDis);
		}
		if(cmd->subshell!=NULL){
			int sub_cmd_status=command_line_exec(cmd->subshell);
			if(sub_cmd_status){
				perror("subshell error");
				abort();
			}
		}
		if((cmd->argv[0]=='\0' && !cmd->subshell) || !strcmp(cmd->argv[0],"exit")){//handle empty command and "exit"
			_exit(EXIT_SUCCESS);
		}
		if(!strcmp(cmd->argv[0],"cd") ){
			if(cmd->argv[2]!='\0'||cmd->argv[1]=='\0'){
                                exit(EXIT_FAILURE);
			}
			if(chdir(cmd->argv[1])<0){
				perror("cd");
				exit(EXIT_FAILURE);
			}
			exit(EXIT_SUCCESS);
		}

		int cmd_status=execvp(cmd->argv[0], cmd->argv);
		if(cmd_status){
			perror("command execvp error");
			abort();
		}
	}
	else{//parent
		if(cmd->controlop==CMD_PIPE){
			close(pipefd[1]);
			*pass_pipefd=pipefd[0];
		}
		if(cmd->argv[0] && !strcmp(cmd->argv[0],"cd")){
			if(cmd->argv[2]!='\0'||cmd->argv[1]=='\0'){
				perror("cd: Syntax error on bad number of arguments\n ");
			}
			if(chdir(cmd->argv[1])<0){
				perror("cd");
			}
		}
		if(cmd->argv[0] && !strcmp(cmd->argv[0],"exit")){
			_exit(EXIT_SUCCESS);
		}
	}
	// return the child process ID
	return pid;
}


/* command_line_exec(cmdlist)
 *
 *   Execute the command list.
 *
 *   Execute each individual command with 'command_exec'.
 *   String commands together depending on the 'cmdlist->controlop' operators.
 *   Returns the exit status of the entire command list, which equals the
 *   exit status of the last completed command.
 *
 *   The operators have the following behavior:
 *
 *      CMD_END, CMD_SEMICOLON
 *                        Wait for command to exit.  Proceed to next command
 *                        regardless of status.
 *      CMD_AND           Wait for command to exit.  Proceed to next command
 *                        only if this command exited with status 0.  Otherwise
 *                        exit the whole command line.
 *      CMD_OR            Wait for command to exit.  Proceed to next command
 *                        only if this command exited with status != 0.
 *                        Otherwise exit the whole command line.
 *      CMD_BACKGROUND, CMD_PIPE
 *                        Do not wait for this command to exit.  Pretend it
 *                        had status 0, for the purpose of returning a value
 *                        from command_line_exec.
 */
int
command_line_exec(command_t *cmdlist)
{
	int cmd_status = 0;	    // status of last command executed
	int pipefd = STDIN_FILENO;  // read end of last pipe
	pid_t ret;
	//int jobID=0;
	while (cmdlist) {
		int wp_status;	    // Hint: use for waitpid's status argument!
				    // Read the manual page for waitpid() to
				    // see how to get the command's exit
				    // status (cmd_status) from this value.

		// TODO: Fill out this function!
		// If an error occurs in command_exec, feel free to abort().
		
		/* Your code here. */
		ret=command_exec(cmdlist, &pipefd);
		if(ret<0){
			perror("command_exec error");
		}
		if(cmdlist->controlop==CMD_AND){
			if(waitpid(ret,&wp_status, 0)==-1){
				perror("CMD_AND waitpid error");
			}
			if(WIFEXITED(wp_status)){
				cmd_status=WEXITSTATUS(wp_status);
				if(cmd_status)
					goto done;
			}
		}
		else if(cmdlist->controlop==CMD_OR){
			if(waitpid(ret,&wp_status, 0)==-1){
				perror("CMD_OR waitpid error");
			}
			if(WIFEXITED(wp_status)){
				cmd_status=WEXITSTATUS(wp_status);
				if(!cmd_status)
					goto done;
			}
		}
		else if(cmdlist->controlop==CMD_END || cmdlist->controlop==CMD_SEMICOLON){
			if(waitpid(ret,&wp_status, 0)==-1){
				perror("CMD_END waitpid error");
			}
			if(WIFEXITED(wp_status)){
				cmd_status=WEXITSTATUS(wp_status);
			}
			/*if(cmdlist->controlop==CMD_END){
				wait(&cmd_status);
			}*/
		}
		else if(cmdlist->controlop==CMD_PIPE || cmdlist->controlop==CMD_BACKGROUND){
			if(waitpid(ret,&wp_status, WNOHANG)==-1){
				perror("CMD_PIPE waitpid error");
			}
			if(cmdlist->controlop==CMD_BACKGROUND){
				report_background_job(jobID++,ret);
			}
			cmd_status=0;
		}

		cmdlist = cmdlist->next;
	}
	

done:
	return cmd_status;
}

void report_background_job(int job_id, int process_id) {
    fprintf(stderr, "[%d] %d\n", job_id, process_id);
}
