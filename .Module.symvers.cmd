cmd_/media/sf_progetto_soa/Module.symvers :=  sed 's/ko$$/o/'  /media/sf_progetto_soa/modules.order | scripts/mod/modpost -m     -o /media/sf_progetto_soa/Module.symvers -e -i Module.symvers -T - 
