from fabric.api import *

env.key_filename = ["/home/scj7t4/.ssh/id_rsa"]
env.warn_only = True

BOOST_ROOT = '~/boost'
BROKER_DIR = '/home/scj7t4/FREEDM/Broker'
SRC_DIR = '/home/scj7t4/FREEDM/Broker/src'

env.key_filename = ["/home/scj7t4/.ssh/id_supercluster"]

def host_type():
    run('uname -s')

def change_and_check():
    with cd('/tmp'):
        run('pwd')

def pull_checkout(branch='master'):
    with cd(SRC_DIR):
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
def start_sim(runtime='10m',timeout='10s'):
    with cd(SRC_DIR):
        cmd = "timeout -k %s %s ./PosixBroker" % (timeout,runtime)
        result = run(cmd)
        if result.return_code != 0:
            print "Test failed with error code."

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
    run("killall %s PosixBroker -u scj7t4" % k)

def get_uuid():
    with cd(SRC_DIR):
        result = run("./PosixBroker -u")
    return str(result).strip()

def setup_sim(expconfig):
    """
    There is currently a bug with fabric.
    with cd(SRC_DIR):
    Doesn't work.
    """
    put(expconfig,'/home/scj7t4/FREEDM/Broker/src/network.xml')

