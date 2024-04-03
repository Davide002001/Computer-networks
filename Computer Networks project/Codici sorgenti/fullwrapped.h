ssize_t FullWrite(int fd, const void *buf, size_t count){
    
    size_t nleft;
    ssize_t nwritten;
    nleft = count;
    while (nleft > 0) {

    
    if ( (nwritten = write(fd, buf, nleft)) < 0) {
        if (errno == EINTR) { 
        continue;
        
        } else {
        exit(nwritten); 
        }
    }

    nleft -= nwritten;
    
    buf +=nwritten;
    
    }

	return (nleft);
}

ssize_t FullRead(int fd, void *buf, size_t count)
{ 
    size_t nleft; 
    ssize_t nread; 
    nleft = count; 
    while (nleft > 0) 
    {
	    if( (nread=read(fd, buf, nleft))<0)
      {
        //se c'è stato errore
		    if(errno=EINTR){ continue; }
		    else{exit(nread);}
	    }else if(nread==0){ break;}//chiuso il canale

	    nleft-=nread;
	    buf+=nread;
    }
      buf=0; 
      return (nleft); 
 }