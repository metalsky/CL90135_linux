echo backup sorgenti linux in /home/bagside/temp
rsync -zvr --max-size='80K' /home/bagside/workdir/lsp/ti-davinci/linux-2.6.18_pro500 /home/bagside/temp
