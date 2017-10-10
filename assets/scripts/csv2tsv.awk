BEGIN {
	FPAT="([^,]*)|(\"([^\"]|\"\")+\"[^,]*)"
}
{
	for(i = 1; i <= NF; i++) {
		if(substr($i, 1, 1) == "\"") {
			gsub(/\"\"/, "\"", $i)
			gsub(/^\"/, "", $i)
			gsub(/\"$/, "", $i)
		}

		printf("%s", $i)

		if(i != NF)
			printf("\t")
	}

	printf("\n")
}
