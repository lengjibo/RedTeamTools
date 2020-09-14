# -*- coding:utf-8 -*-
import time
import json
import requests
from operator import itemgetter
from bs4 import BeautifulSoup
from translate import Translator

class SearchCVE:
    def __init__(self, CVE):
        self.CVE = CVE
        self.url = "https://nvd.nist.gov/vuln/detail/{}".format(self.CVE)
        self.context = ""
        self.score = ""
    def fanyi(self, context):
        translator = Translator(to_lang="chinese")
        translation = translator.translate(context)
        return translation

    def run(self):
        self.get_cve_description()
        if self.context != "未查询到CVE信息":
            self.context = self.fanyi(self.context)
        if self.score != "暂无":
            self.score = self.fanyi(self.score)

        return self.score, self.context

    def get_cve_description(self):

        response = requests.get(self.url).text
        soup = BeautifulSoup(response, "lxml")

        description = soup.body.findAll('p', {'data-testid': "vuln-description"})
        self.context = description[0].text.replace("** DISPUTED **", "") if len(description) == 1 else "未查询到CVE信息".decode('utf-8')

        cvss = soup.body.findAll('a', {'data-testid': "vuln-cvss3-panel-score"})
        self.score = cvss[0].text if len(cvss) == 1 else "暂无".decode('utf-8')


def getNews():
    year = time.strftime("%Y", time.localtime(time.time()))
    try:
       api = f"https://api.github.com/search/repositories?q=CVE-{year}&sort=updated"
       response = requests.get(api).text
       data = json.loads(response)
       return data
    except Exception as e:
       print(e, "Github链接不通")


def CveName(index):
    item = items[index]
    cve_name = item['name']
    cve_url = item['svn_url']
    return cve_name,cve_url


def sendMsg(content,score,context,cve_url):
    send_url = f"https://sc.ftqq.com/{SCKEY}.send"
    context = "　CVE编号：" + content + '\n\n' + "　漏洞评分："+ score + '\n\n' +"　漏洞详情：" + context + '\n\n' +"　POC地址："+cve_url
    data = {
        "text": "CVE监控提醒",
        "desp": context
    }
    r = requests.post(send_url, data=data)


if __name__ == '__main__':
    SCKEY = ""
    total = 0  # 初始化
    title = '''
  ____           ____                      _     
 / ___|_   _____/ ___|  ___  __ _ _ __ ___| |__  
| |   \ \ / / _ \___ \ / _ \/ _` | '__/ __| '_ \ 
| |___ \ V /  __/___) |  __/ (_| | | | (__| | | |
 \____| \_/ \___|____/ \___|\__,_|_|  \___|_| |_|

                                        by lengyi@honghusec
            '''
    print(title)
    while True:
        data = getNews()
        if total != data['total_count']:
            total = data['total_count']
            items = sorted(data['items'], key=itemgetter('id'), reverse=True) 
            content,cve_url = CveName(0)  
            s = SearchCVE(content)
            score, context = s.run()
            print('[+]CVE编号：' + content )
            print('[+]CVE评分：' + score )
            print('[+]漏洞描述：' + context )
            print('[+]POC地址：' + cve_url )
            sendMsg(content,score,context,cve_url)
        time.sleep(60)
