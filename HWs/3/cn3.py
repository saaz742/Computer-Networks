#sara azarnoush 98170668
#hw3 computer network

import subprocess
import shlex


# block in/out traffic to ip, domain regex url, port
def block_ip_url_port():
    ip = input("Enter ip: ")
    url = input("Enter url: ")
    port = input("Enter port: ")
    io = input("Enter in/out: ")
    type, ip_flag, compip, compurl, compport = ""
    if(io.lower() == "in"):
        ip_flag = "-s"
        type = "INPUT"
    else:
        ip_flag = "-d"
        type = "OUTPUT"
    if ip == "":
        compip = ip_flag + " " + ip
    if url == "":
        compurl = "-m string --algo bm --string \"" + url + "\""
    if port == "":
        subprocess.call(shlex.split(
            'iptables -A {} {} {} -j DROP'.format(type, compip, compurl)))
    elif(url != "" and ip != ""):
        compport = "--dport " + port
        subprocess.call(shlex.split(
            'iptables -A {} -p tcp {} {} {} -j DROP'.format(type, compip, compport, compurl)))
        subprocess.call(shlex.split(
            'iptables -A {} -p udp {} {} {} -j DROP'.format(type, compip, compport, compurl)))
    else:
        print("all empty")


# block in/out traffic base on count, protocol, request type
def block_count_protocol_req():
    inp = input("Enter protocol (FTP, SSH, DNS, HTTP, SMTP, DHCP): ")
    while (not inp.lower() == "exit"):
        type = input("Enter request type (INPUT, OUTPUT): ")
        if(inp.lower() == "ftp"):
            file =  "\"" + input("Enter file: ") +  "\""
            subprocess.call(shlex.split(
                'iptables -A {} -p tcp --dport 21 -m string --algo bm --string {} -j DROP'.format(type,file)))
        elif(inp.lower() == "ssh"):
            user = input("Enter user ip: ")
            subprocess.call(shlex.split(
                'iptables -A {} -i {} -p tcp --dport ssh -j DROP'.format(type,user)))
            subprocess.call(shlex.split("systemctl restart ssh.service"))
        elif(inp.lower() == "dns"): #msg
            url = "\"" + input("Enter URL: ") + "\""
            subprocess.call(shlex.split(
                'iptables -A {} -p udp --dport 53 -m string --algo bm --string {} -j REJECT'.format(type,url)))
            subprocess.call(shlex.split("systemd-resolve --flush-caches"))
        elif(inp.lower() == "DHCP"): #msg
            mac = "\"" + input("Enter mac: ") + "\""
            subprocess.call(shlex.split(
                'iptables -A {} -p udp --sport 68 --dport 67 -m mac --mac-source {} -j REJECT'.format(type,mac)))
        elif(inp.lower() == "http"):
            val =  "\"" + input("Enter value: ") + "\""
            subprocess.call(shlex.split(
                'iptables -A {} -p tcp --dport 80 -m string --algo bm --string {} -j DROPP'.format(type,val)))
            subprocess.call(shlex.split(
                'iptables -A {} -p tcp --dport 443 -m string --algo bm --string {} -j DROPP'.format(type,val)))
        #or use multiport 
        #    subprocess.call(shlex.split(
        #       'iptables -A {} -p tcp -m multiport --dport 80,443 -m string --algo bm --string {} -j DROPP'.format(type,val)))
        elif(inp.lower() == "smtp"):
            email = "\"" +  input("Enter email: ") + "\""
            subprocess.call(shlex.split(
                'iptables -A {} -p tcp --dport 25 -m string --algo bm --string {} -j DROP'.format(type,email)))
        else:
            print("Invalid command")
        inp = input("Enter command: ")

# block header
def block_header():
    key = input("Enter key: ")
    value = input("Enter value: ")
    header = "\"" + key + ":" + value + "\"" 
    subprocess.call(shlex.split(
        'iptables -A INPUT -m string --algo bm --string {} -j DROP'.format(header)))
    subprocess.call(shlex.split(
        'iptables -A OUTPUT -m string --algo bm --string {}'.format(header)))

#https://www.provgn.com/knowledgebase/4/Filtering-DDoS-Attacks-with-IPTables-.html
#https://javapipe.com/blog/iptables-ddos-protection/
#https://www.cyberciti.biz/tips/howto-limit-linux-syn-attacks.html
#https://itgala.xyz/iptables-antiddos-protection/

#DoS attack
def def_dos():
    # ANTI DDOS
    subprocess.call(shlex.split('iptables -A FORWARD -p tcp --syn -m limit --limit 1/second -j ACCEPT'))
    subprocess.call(shlex.split('iptables -A FORWARD -p udp -m limit --limit 1/second -j ACCEPT'))
    subprocess.call(shlex.split('iptables -A FORWARD -p icmp --icmp-type echo-request -m limit --limit 1/second -j ACCEPT'))
    subprocess.call(shlex.split('iptables -A FORWARD -p tcp --tcp-flags SYN,ACK,FIN,RST RST -m limit --limit 1/s -j ACCEPT'))
    subprocess.call(shlex.split('iptables -A INPUT -p tcp -m tcp --tcp-flags RST RST -m limit --limit 2/second --limit-burst 2 -j ACCEPT'))
    # SYNPROXY rules that help mitigate SYN floods
    subprocess.call(shlex.split('iptables -t raw -A PREROUTING -p tcp -m tcp --syn -j CT --notrack'))
    subprocess.call(shlex.split('iptables -A INPUT -p tcp -m tcp -m conntrack --ctstate INVALID,UNTRACKED -j SYNPROXY --sack-perm --timestamp --wscale 7 --mss 1460'))
    subprocess.call(shlex.split('iptables -A INPUT -m conntrack --ctstate INVALID -j DROP'))
    #not syn
    subprocess.call(shlex.split('iptables -t mangle -A PREROUTING -p tcp ! --syn -m conntrack --ctstate NEW -j DROP'))
    #slowloris 
    subprocess.call(shlex.split('iptables -I INPUT -p tcp --dport 80 -m connlimit --connlimit-above 50 --connlimit-mask 20 -j DROP'))
    #dns flood
    subprocess.call(shlex.split('iptables -A INPUT -p tcp -m connlimit --connlimit-above 80 -j REJECT --reject-with tcp-reset'))
    subprocess.call(shlex.split('iptables -A INPUT -p tcp --tcp-flags RST RST -m limit --limit 2/s --limit-burst 2 -j ACCEPT'))
    subprocess.call(shlex.split('iptables -A INPUT -p tcp --tcp-flags RST RST -j DROP'))
    change = input("Do you want to change more than the default settings? (y/n): ")
    if change == "y":
        change_dos()

def change_dos():
    port = input("Enter port: ")
    time = input("Enter time: ")
    number = input("Enter number: ")
    cnt = input("Enter cnt: ")
    limit = input("Enter limit: ")
    #slowloris 
    subprocess.call(shlex.split('iptables -I INPUT -p tcp --dport {} -m connlimit --connlimit-above {} --connlimit-mask 20 -j DROP'.format(port, number)))
    #dns flood
    subprocess.call(shlex.split('iptables -A INPUT -p tcp -m connlimit --connlimit-above {} -j REJECT --reject-with tcp-reset'.format(number)))
    subprocess.call(shlex.split('iptables -A INPUT -p tcp --tcp-flags RST RST -m limit --limit {}/s --limit-burst {} -j ACCEPT'.format(limit,limit)))
    subprocess.call(shlex.split('iptables -A INPUT --match state --state NEW --match recent --update --seconds {} --hitcount {} --jump DROP'.format(time, cnt)))



# use DB to optimize defend agains DoS
def def_dos_db():
    #allows MySQL database server access (202.54.1.20) from Apache web server (202.54.1.50)
    subprocess.call(shlex.split('iptables -A INPUT -p tcp -s 202.54.1.50 --sport 1024:65535 -d 202.54.1.20 --dport 3306 -m state --state NEW,ESTABLISHED -j ACCEPT'))
    subprocess.call(shlex.split('iptables -A OUTPUT -p tcp -s 202.54.1.20 --sport 3306 -d 202.54.1.50 --dport 1024:65535 -m state --state ESTABLISHED -j ACCEPT'))
    #allow outgoing MySql client request (made via mysql command line client or perl/php script), from firewall host 202.54.1.20 
    subprocess.call(shlex.split('iptables -A OUTPUT -p tcp -s 202.54.1.20 --sport 1024:65535 -d 0/0 --dport 3306 -m state --state NEW,ESTABLISHED -j ACCEPT'))
    subprocess.call(shlex.split('iptables -A INPUT -p tcp -s 0/0 --sport 3306 -d 202.54.1.20 --dport 1024:65535 -m state --state ESTABLISHED -j ACCEPT'))
    # QUERY
    subprocess.call(shlex.split('iptables -t filter -A OUTPUT -p tcp --dport 10011 -j ACCEPT'))
    subprocess.call(shlex.split('iptables -t filter -A OUTPUT -p tcp --dport 30033 -j ACCEPT'))
    subprocess.call(shlex.split('iptables -t filter -A INPUT -p tcp --dport 10011 -j ACCEPT'))
    subprocess.call(shlex.split('iptables -t filter -A INPUT -p tcp --dport 30033 -j ACCEPT'))
    #use php ... to connect to mysql and get data there isnt a iptable command for it

# block port scanner and port knocking
def block_port_scanner_knocking():
    inp = input("Enter scan/knock: ")
    while (not inp.lower() == "exit"):
        if(inp.lower() == "scan"):
            ports = input("Enter porsts(seprate with ,): ")
            for port in ports.split(","):
                subprocess.call(shlex.split(
                    'iptables -A INPUT -p tcp --dport {} -m recent --set'.format(port)))
                subprocess.call(shlex.split(
                    'iptables -A INPUT -p tcp --dport {} -m recent --update --seconds 60 --hitcount 3 -j DROP'.format(port)))
        elif(inp.lower() == "knock"):
            chain = input("Enter chain: ")
            ports = input("Enter 4 port last one is the target(seprate with ,): ").split(",")
            subprocess.call(shlex.split('iptables -N {}'.format(chain)))
            subprocess.call(shlex.split(
                'iptables -A INPUT -j {}'.format(chain)))
            subprocess.call(shlex.split(
                'iptables -A {} -p tcp --dport {} -m recent --rcheck --seconds 60 --reap --name knockfinal -j ACCEPT'.format(chain, ports[3])))
            subprocess.call(shlex.split(
                'iptables -A {} -p tcp -m tcp --dport {} -m recent --set --name knock1 -j REJECT'.format(chain, port[0])))
            subprocess.call(shlex.split(
                'iptables -A {} -p tcp -m recent --rcheck --seconds 10 --reap --name knock1 -m tcp --dport {} -m recent --set --name knock2 -j REJECT'.format(chain, port[1])))
            subprocess.call(shlex.split(
                'iptables -A {} -p tcp -m recent --rcheck --seconds 10 --reap --name knock2 -m tcp --dport {} -m recent --set --name knockfinal -j REJECT'.format(chain, port[2])))
            subprocess.call(shlex.split(
                'iptables -A INPUT -p tcp --dport {} -m state --state NEW,INVALID -j REJECT'.format(port[3])))
        else:
            print("Invalid command")
        inp = input("Enter command: ")

#all rules
def all():
    subprocess.call(shlex.split('iptables -S'))
    subprocess.call(shlex.split('iptables -L -v -n'))

#Set the default policies to allow everything
def defult():
    subprocess.call(shlex.split('iptables -P INPUT ACCEPT'))
    subprocess.call(shlex.split('iptables -P OUTPUT ACCEPT'))
    subprocess.call(shlex.split('iptables -P FORWARD ACCEPT'))

#flush
def flush():
    subprocess.call(shlex.split('iptables -F'))

#Zero the packet and byte counters in all chains
def zero():
    subprocess.call(shlex.split('IPTABLES -t filter -Z'))
    subprocess.call(shlex.split('IPTABLES -t nat -Z'))
    subprocess.call(shlex.split('IPTABLES -t mangle -Z'))
    subprocess.call(shlex.split('IPTABLES -t raw -Z'))

def help():
    help = [
        "block in/out traffic to ip, domain regex url, port: iup",
        "block in/out traffic base on count, protocol, request type: cpr",
        "block header: head",
        "DoS attack: dos",
        "use DB to optimize defend agains DoS: db",
        "block port scanner and port knocking: sk",
        "**** more ****",
        "default policies: defult",
        "all rules: all",
        "zero packet and byte: zero",
        "flush: flush",
        "help: help",
        "exit: exit"
    ]
    for i in help:
        print(i)


def CommandProcessor():
    help()
    inp = input("Enter command: ")
    while (not inp.lower() == "exit"):
        if(inp.lower() == "iup"):
            block_ip_url_port()
        elif(inp.lower() == "cpr"):
            block_count_protocol_req()
        elif(inp.lower() == "head"):
            block_header()
        elif(inp.lower() == "dos"):
            def_dos()
        elif(inp.lower() == "db"):
            def_dos_db()
        elif(inp.lower() == "sk"):
            block_port_scanner_knocking()
        elif(inp.lower() == "defult"):
            defult()
        elif(inp.lower() == "all"):
            all()
        elif(inp.lower() == "zero"):
            zero()
        elif(inp.lower() == "flush"):
            flush()
        elif(inp.lower() == "help"):
            help()
        else:
            print("Invalid command")
        inp = input("Enter command: ")


if __name__ == '__main__':
    CommandProcessor()
