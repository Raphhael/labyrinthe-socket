all:
	echo "Répertoire source : ./source"
	echo "Répertoire des executables : ./bin"
	$(MAKE) -C ./bin

client:
	$(MAKE) -C ./source/client client
serveur: 
	$(MAKE) -C ./source/serveur
import: 
	$(MAKE) -C ./source/import
