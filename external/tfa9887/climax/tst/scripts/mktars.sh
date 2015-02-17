echo making links and deref tar
tar  czf  nxptfa_links.tgz nxptfa --exclude nxptfa/.sconsign.dblite
tar  hczf  nxptfa_deref.tgz nxptfa --exclude nxptfa/.cproject  --exclude nxptfa/.project  --exclude nxptfa/.sconsign.dblite
