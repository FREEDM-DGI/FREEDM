"""
@author Stephen Jackson <scj7t4@mst.edu>
@compiler python2
@project FREEDM Deployment
@description This class creates a psuedo-generator for network settings for
    firing off many short runs of various network settings rapidly.
@license These source code files were created at as part of the
    FREEDM DGI Subthrust, and are intended for use in teaching or
    research.  They may be freely copied, modified and redistributed
    as long as modified versions are clearly marked as such and
    this notice is not removed.

    Neither the authors nor the FREEDM Project nor the
    National Science Foundation
    make any warranty, express or implied, nor assumes
    any legal responsibility for the accuracy,
    completeness or usefulness of these codes or any
    information distributed with these codes.

    Suggested modifications or questions about these codes 
    can be directed to Dr. Bruce McMillin, Department of 
    Computer Science, Missouri University of Science and
    Technology, Rolla, MO  65409 (ff@mst.edu).
"""
import network

class Experiment(object):
    """
    An object which can be iterated to produce various network settings files.
    Given a set of hostnames and a granularity, this script will produce fileIO
    descriptors which can be written to network.xml files to simulate various
    network settings.
    """
    def __init__(self,host2uuid,granularity):
        """
        Intializes the experiment object by generating an experiment counter.
        
        @param host2uuid A dictionary mapping hostnames to UUIDs
        @param granularity How much to step for each experiment
        """
        self.expcounter = [0] * ((len(host2uuid) * (len(host2uuid)-1))/2)
        self.host2uuid = host2uuid
        self.granularity = granularity
        self.fixed = dict()
        self.bussed = []

    def bus_edges(self,hostpairs):
        #Translate host pairs into indicies:
        tmp = []
        for x in hostpairs:
            tmp.append(self.get_index(x[0],x[1]))
        #For simplicity, let us sort tmp
        tmp.sort()
        self.bussed += [tmp]

    def get_index(self,host1,host2):
        index = 0
        seen = []
        tuuidx = self.host2uuid[host1]
        tuuidy = self.host2uuid[host2]
        for (hostx,uuidx) in self.host2uuid.iteritems():
            for (hostx,uuidy) in self.host2uuid.iteritems():
                if uuidx == uuidy:
                    continue
                if uuidy in seen:
                    continue
                if (tuuidx == uuidx and tuuidy == uuidy) or (tuuidx == uuidy and tuuidy == uuidx):
                    return index
                index += 1
                seen.append(uuidx)
        return None

    def fix_edge(self,host1,host2,value):
        """
        When called, sets the edge between host1 and host2 to have a fixed
        value In the current experiment and all those that follow, that
        network connection will remain at the value specified.

        Although the freedm.xml file supports more complex network bus
        descriptions (such as unsymmetric channels) this software only
        supports generating symmertric channels.
        
        @param host1 The first host to as source vertex.
        @param host2 The second host to use a destination vertex.
        @param value The reliability value to set the channel to. (0-100)
        """
        index = self.get_index(host1,host2)
        seen = []
        tuuidx = self.host2uuid[host1]
        tuuidy = self.host2uuid[host2]
        self.fixed[(tuuidx,tuuidy)] = (index,value)
        self.expcounter[index] = value

    def maptonetwork(self):
        """
        This function will return a dictionary of dictionaries, which provides
        a table of the network settings for the current step of the experiment.
        
        @return A dictionary of dictionaries keyed as [sourcehost][desthost]
            whose value is the reliability of that network edge.
        """
        out = dict()
        index = 0
        seen = []
        for (hostx,uuidx) in self.host2uuid.iteritems():
            out.setdefault(hostx,dict())
            for (hosty,uuidy) in self.host2uuid.iteritems():
                out.setdefault(hosty,dict())
                if uuidx == uuidy:
                    continue
                if uuidy in seen:
                    continue
                assert not hosty in out[hostx], 'Key %s already in %s' % (hosty,hostx)
                assert not hostx in out[hosty], 'Key %s already in %s' % (hostx,hosty)
                out[hostx].setdefault(hosty, self.expcounter[index])
                out[hosty].setdefault(hostx, self.expcounter[index])
                index += 1
                seen.append(uuidx)
        assert index == len(self.expcounter), 'Did not use all exp settings'
        return out

    def next(self,default=None):
        """
        Increments the experiment counter. If there are no more experiments
        left to be run, default will be returned (or None) if default has
        not been provided
        
        @param default the value to return if there are no more experiments
            to produce.
        @ereturn the new value of expcounter
        """
        index = 0
        max_v = [100]*(len(self.expcounter))
        bus_onto = []
        for (key,(indexa,value)) in self.fixed.iteritems():
            max_v[indexa] = value
        if self.expcounter == max_v:
            return default
        while index < len(self.expcounter):
            if index in [indexa for (key,(indexa,value)) in self.fixed.iteritems()]:
                #Don't change values we've fixed.
                index += 1
                continue
            #If this edge is in a bussed set and is the first item, iterate it and
            #update all the other bussed pairs, otherwise, continue
            in_bus = False
            for bus in self.bussed:
                if len(bus) > 0 and bus[0] != index and index in bus:
                    in_bus = True
                    break
                for x in bus[1:]:
                    bus_onto.append(x)
            if in_bus:
                index += 1
                continue
            if self.expcounter[index] == 100:
                self.expcounter[index] = 0
                for x in bus_onto:
                    self.expcounter[x] = 0
                index += 1
            elif self.expcounter[index] < 100:
                self.expcounter[index] += self.granularity
                if self.expcounter[index] > 100:
                    self.expcounter[index] = 100
                for x in bus_onto:
                    self.expcounter[x] = self.expcounter[index]
                break
        for x in self.expcounter:
            assert x <= 100, "Experiment has overrun"
        return self.expcounter

    def __repr__(self):
        """
        Allows converting this object to a string. Debugging purposes only;
        this does not serialize the object.
        
        @return The experiment counter as a string.
        """
        return str(self.expcounter)
    def tsv_head(self):
        """
        Generates the header for a TSV experiment data file.

        @preturn The header for the experiment data.
        """
        seen = []
        out = []
        for (hostx,uuidx) in self.host2uuid.iteritems():
            for (hosty,uuidy) in self.host2uuid.iteritems():
                if uuidy in seen:
                    continue
                if uuidx == uuidy:
                    continue
                if len(out) == 0:
                    tmp = "#"
                else:
                    tmp = ""
                tmp +="%sx%s" % (hostx,hosty)
                out.append(tmp)
                seen.append(uuidx)
        return "\t".join(out)
    def tsv_entry(self):
        """
        Generates an entry of the experiment tsv file.
        
        @return a tab seperated string of the current experiment settings.
        """
        return "\t".join([str(x) for x in self.expcounter])
    def generate_files(self):
        """
        Generates the all the file descriptors for the all the network.xml
        files for the current stage of the experiment.
        
        @return A dictionary of fileio objects that can be written into the
            hosts network.xml files.
        """
        filedict = dict()
        m = self.maptonetwork()
        for (k,v) in m.iteritems():
            d = dict()
            for(k2,v2) in v.iteritems():
                # make a new dict, translating hostnames to uuids
                d.setdefault(self.host2uuid[k2],v2)
            filedict.setdefault(k,network.create_network_file(100,d))
        return filedict    

if __name__ == "__main__":
    import random
    dictthing = {'google.com':'goog','amazon.com':'amaz','movingpictures.com':'mvpc','rush.com':'rush'}
    x = Experiment(dictthing,20)
    i = 0
    
    x.fix_edge('google.com','amazon.com',27)
    x.fix_edge('movingpictures.com','rush.com',73)

    x.fix_edge('google.com','movingpictures.com',0)
    x.fix_edge('amazon.com','rush.com',0)
        
    x.bus_edges([('google.com','rush.com'),('amazon.com','movingpictures.com')])    

    if 1:
        while x.next() != None:
            print x
            i += 1
        print i
        raw_input("Press Enter to continue...")
        x = Experiment(dictthing,7)
        x.fix_edge('google.com','amazon.com',27)
        x.fix_edge('movingpictures.com','rush.com',73)

        x.fix_edge('google.com','movingpictures.com',0)
        x.fix_edge('amazon.com','rush.com',0)
            
        x.bus_edges([('google.com','rush.com'),('amazon.com','movingpictures.com')])    
        i = 0
        while x.next() != None:
            print x
            i += 1
        print i
        raw_input("Press Enter to continue...")
    while 1:
        x = Experiment(dictthing,20)
        x.fix_edge('google.com','amazon.com',27)
        x.fix_edge('movingpictures.com','rush.com',73)

        x.fix_edge('google.com','movingpictures.com',0)
        x.fix_edge('amazon.com','rush.com',0)
            
        x.bus_edges([('google.com','rush.com'),('amazon.com','movingpictures.com')])    
        for i in xrange(random.randint(1,40000)):
            x.next()
        print x.generate_files()
        m = x.maptonetwork()
        for (k,v) in m.iteritems():
            for (k2,v2) in v.iteritems():
                if m[k][k2] != m[k2][k]:
                    print "BAD."
                    exit(1)

