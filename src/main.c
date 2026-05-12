void lsh_loop(void){
    char *line;
    char **args;
    int status;

    do{
        printf("> ");
        line = lsh_read();
        args = lsh_split();
        status = lsh_execute(args);

        free(line);
        free(status);
    } while (status);   //coment svsfssd
    
}

int main(int argc, char **argv){
    // load config files, if any

    // Run command loop
    lsh_loop();

    // Perform any shutdown/cleanup

    return EXIT_SUCCESS;
}