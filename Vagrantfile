Vagrant.configure("2") do |config|
	config.vm.box = "ubuntu/trusty64"
	config.vm.synced_folder "./", "/root/NoLAG/"
	config.vm.provision :shell, path: "requirements.sh"

	config.vm.define "node1" do |node1|
    # node1
	node1.vm.network "private_network", ip: "192.168.200.2"
	node1.vm.network "private_network", ip: "192.168.200.3"
  	end

	config.vm.define "node2" do |node2|
    # node2
	node2.vm.network "private_network", ip: "192.168.200.5"
	node2.vm.network "private_network", ip: "192.168.200.6"
  	end
end
