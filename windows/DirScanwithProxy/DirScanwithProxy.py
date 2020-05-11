import requests
from queue import Queue
import sys,random,re
import threading

USERAGENT = [
    "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.1 (KHTML, like Gecko) Chrome/22.0.1207.1 Safari/537.1",
    "Mozilla/5.0 (X11; CrOS i686 2268.111.0) AppleWebKit/536.11 (KHTML, like Gecko) Chrome/20.0.1132.57 Safari/536.11",
    "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/536.6 (KHTML, like Gecko) Chrome/20.0.1092.0 Safari/536.6",
    "Mozilla/5.0 (Windows NT 6.2) AppleWebKit/536.6 (KHTML, like Gecko) Chrome/20.0.1090.0 Safari/536.6",
    "Mozilla/5.0 (Windows NT 6.2; WOW64) AppleWebKit/537.1 (KHTML, like Gecko) Chrome/19.77.34.5 Safari/537.1",
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.9 Safari/536.5",
    "Mozilla/5.0 (Windows NT 6.0) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.36 Safari/536.5",
    "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/536.3 (KHTML, like Gecko) Chrome/19.0.1063.0 Safari/536.3",
    "Mozilla/5.0 (Windows NT 5.1) AppleWebKit/536.3 (KHTML, like Gecko) Chrome/19.0.1063.0 Safari/536.3",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_8_0) AppleWebKit/536.3 (KHTML, like Gecko) Chrome/19.0.1063.0 Safari/536.3",
    "Mozilla/5.0 (Windows NT 6.2) AppleWebKit/536.3 (KHTML, like Gecko) Chrome/19.0.1062.0 Safari/536.3",
    "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/536.3 (KHTML, like Gecko) Chrome/19.0.1062.0 Safari/536.3",
    "Mozilla/5.0 (Windows NT 6.2) AppleWebKit/536.3 (KHTML, like Gecko) Chrome/19.0.1061.1 Safari/536.3",
    "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/536.3 (KHTML, like Gecko) Chrome/19.0.1061.1 Safari/536.3",
    "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/536.3 (KHTML, like Gecko) Chrome/19.0.1061.1 Safari/536.3",
    "Mozilla/5.0 (Windows NT 6.2) AppleWebKit/536.3 (KHTML, like Gecko) Chrome/19.0.1061.0 Safari/536.3",
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/535.24 (KHTML, like Gecko) Chrome/19.0.1055.1 Safari/535.24",
    "Mozilla/5.0 (Windows NT 6.2; WOW64) AppleWebKit/535.24 (KHTML, like Gecko) Chrome/19.0.1055.1 Safari/535.24"
]

REFERERS = [
    "https://www.baidu.com",
    "http://www.baidu.com",
    "https://www.google.com.hk",
    "http://www.so.com",
    "http://www.sogou.com",
    "http://www.soso.com",
    "http://www.bing.com",
]

def get_proxy_ip(ip_queue):
    proxy_url = "http://www.66ip.cn/nmtq.php?getnum=10000"
    headers = {
        'User-Agent': random.choice(USERAGENT),
        'Accept': 'Accept:text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
        'Cache-Control': 'max-age=0',
        'referer': random.choice(REFERERS),
        'Accept-Charset': 'GBK,utf-8;q=0.7,*;q=0.3',
    }

    resp = requests.get(proxy_url, headers=headers)
    if resp.status_code == 200:
        proxy_url_list = re.findall("(?:[0-9]{1,3}\.){3}[0-9]{1,3}:*[0-9]*", resp.text)
        for proxy_url in proxy_url_list:
            proxy = {}
            proxy['https'] = proxy_url
            # print(proxy)
            try:
                resp = requests.head(url="http://www.baidu.com/", timeout=3, headers=headers, verify=False, allow_redirects=False, proxies=proxy)
                if resp.status_code == 200:
                    ip_queue.put(proxy)
            except:
                pass
    else:
        print('代理ip网站非200，获取代理ip失败!')
        exit(0)


# 多线程实现Web目录扫描
class DirScan(threading.Thread):

    def __init__(self, queue,ip):
        threading.Thread.__init__(self)
        self._queue = queue
        self.ip = ip

    def run(self):
        # 获取队列中的URL
        while not self._queue.empty():
            url = self._queue.get()
            ip = self.ip.get()
            try:
                headers = {
                            'User-Agent': random.choice(USERAGENT),
                            'Accept': 'Accept:text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
                            'Cache-Control': 'max-age=0',
                            'referer': random.choice(REFERERS),
                            'Accept-Charset': 'GBK,utf-8;q=0.7,*;q=0.3',
                }
                # 发送请求
                r = requests.get(url=url, headers=headers, timeout=8,allow_redirects=False,proxies=ip)

                # Web目录存在
                if r.status_code == 200:
                    sys.stdout.write('[+]code 200 : %s\n' % url)
                elif r.status_code == 403:
                    sys.stdout.write('[+]code 403 : %s\n' % url)
                elif r.status_code == 401:
                    sys.stdout.write('[+]code 401 : %s\n' % url)

            except:
                pass


# 定义队列及放入URL
def start(url, ext, count,ip):
    queue = Queue()

    f = open('%s.txt' % ext, 'r')
    for i in f:
        queue.put(url + i.rstrip('\n'))
    # 多线程
    threads = []
    thread_count = int(count)

    for i in range(thread_count):
        threads.append(DirScan(queue,ip))
    for t in threads:
        t.start()
    for t in threads:
        t.join()


# 主函数
if __name__ == '__main__':
    url = 'http://www.xxx.com/'
    queue = Queue()
    print("\033[31m[+]获取代理Ing....")
    threading.Thread(target=get_proxy_ip, args=(queue,)).start()
    print("\033[31m[+]代理获取完成....")
    print("\033[31m[+]开始扫描目录Ing....")
    ext = 'db'
    count = 10
    start(url, ext, count,queue)
