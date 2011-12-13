from fabric.api import *
from os.path import join

#env.key_filename = ["/home/scj7t4/.ssh/id_rsa"]
env.warn_only = True

def host_type():
    run('uname -s')

def change_and_check():
    with cd('/tmp'):
        run('pwd')

def pull_checkout(path,branch='master'):
    with cd(join(path,"Broker/src/")):
        result = run('git branch | grep %s' % branch)
        if result.return_code != 0:
            result = run('git branch -r | grep %s' % branch)
            if result.return_code != 0:
                run('git checkout --track -b %s origin/%s' % (branch,branch))
            else:
                print "Couldn't get into the specified branch! (Not Found)"
                exit(1)
        else:
            run('git checkout %s' % branch)
        run('git pull')
 
def build():
    with cd(BROKER_DIR):
        run("BOOST_ROOT='%s' cmake ." % BOOST_ROOT)
    with cd(SRC_DIR):
        run("BOOST_ROOT='%s' make" % BOOST_ROOT)

@parallel
def start_sim(path):
    with cd(join(path,"Broker/src/")):
        cmd = "./PosixBroker"
        result = run(cmd)
        if result.return_code != 0:
            print "Test failed with error code."

def start_linehost(path):
    with cd(join(path,"PSCAD-Interface/src/")):
        cmd = "./driver"
        run(cmd)

def wait_sim(runtime='10m'):
    print "Waiting for test."
    local("sleep %s" % runtime)
    print "Done Waiting"

@parallel
def end_sim(force=False):
    if force:
        k = "-9"
    else:
        k = ""
    run("killall %s PosixBroker" % k)

def get_uuid(path):
    with cd(join(path,"Broker/src/")):
        result = run("./PosixBroker -u")
    return str(result).strip()

def setup_sim(path,expconfig):
    """
    There is currently a bug with fabric.
    with cd(SRC_DIR):
    Doesn't work.
    """
    put(expconfig, join(path,"/Broker/src/network.xml"))

