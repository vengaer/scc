class config {
    static final int CI_JOB_FUZZTIME = 30
    static final int CI_JOB_FUZZRUNS = 1
    static final int FUZZ_JOB_FUZZTIME = 360
    static final int FUZZ_JOB_FUZZRUNS = 20
}

ccs = ["gcc", "clang"]
buildtypes = ["ci", "fuzz"]
fuzztargets = ["hashtab", "rbtree", "svec"]

fuzztime = config.CI_JOB_FUZZTIME
fuzzruns = config.CI_JOB_FUZZRUNS

pipeline {
    agent none
    environment {
        IMAGE="scc/build"
        ARTIFACTS="artifacts"
        CORPORA="corpora.zip"
    }
    stages {
        stage("Prepare Environment") {
            agent none
            steps {
                script {
                    if(env.BUILDTYPE) {
                        if(!buildtypes.contains(env.BUILDTYPE)) {
                            error("Invalid buildtype ${BUILDTYPE}")
                        }
                        else if(env.BUILDTYPE == "fuzz") {
                            fuzztime = config.FUZZ_JOB_FUZZTIME
                            fuzzruns = config.FUZZ_JOB_FUZZRUNS
                        }
                    }
                }
            }
        }
        stage("Gitlab Pending") {
            steps {
                echo "Notifying Gitlab"
                updateGitlabCommitStatus name: "build", state: "pending"
            }
        }
        stage("Docker Image") {
            agent any
            steps {
                echo "-- Docker Image --"
                sh "docker build -f Dockerfile -t ${IMAGE} ."
            }
        }
        stage("Dynamic Build") {
            when {
                beforeAgent true
                expression {
                    return env.BUILDTYPE != "fuzz"
                }
            }
            agent {
                docker {
                    image "${IMAGE}"
                }
            }
            steps {
                script {
                    ccs.each { cc ->
                        stage("Build ${cc}") {
                            echo "-- Starting ${cc} Build --"
                            sh "CC=${cc} make -B -j\$(nproc)"
                        }
                    }
                }
            }
        }
        stage("Dynamic Test") {
            when {
                beforeAgent true
                expression {
                    return env.BUILDTYPE != "fuzz"
                }
            }
            agent {
                docker {
                    image "${IMAGE}"
                }
            }
            steps {
                script {
                    ccs.each { cc ->
                        stage("Test ${cc}") {
                            echo "-- Running ${cc} Tests --"
                            sh "CC=${cc} make -B check"
                        }
                    }
                }
            }
        }
        stage("Docs") {
            when {
                beforeAgent true
                expression {
                    return env.BUILDTYPE != "fuzz"
                }
            }
            agent {
                docker {
                    image "${IMAGE}"
                }
            }
            steps {
                script {
                    sh "make doc"
                }
            }
        }
        stage("Lint") {
            when {
                beforeAgent true
                expression {
                    return env.BUILDTYPE != "fuzz"
                }
            }
            agent {
                docker {
                    image "${IMAGE}"
                }
            }
            steps {
                script {
                    sh "make -j\$(nproc) lint"
                }
            }
        }
        stage("Fetch Corpora") {
            agent {
                docker {
                    image "${IMAGE}"
                }
            }
            steps {
                echo "Fetching corpora"
                copyArtifacts filter: "${ARTIFACTS}/${CORPORA}", projectName: "${JOB_NAME}", fingerprintArtifacts: true, optional:true
                script {
                    if(fileExists("${ARTIFACTS}/${CORPORA}")) {
                        unzip zipFile: "${ARTIFACTS}/${CORPORA}", dir: "${ARTIFACTS}/prev"
                    }
                    else {
                        echo "No corpora found"
                    }
                }
            }
        }
        stage("Merge Corpora") {
            agent {
                docker {
                    image "${IMAGE}"
                }
            }
            steps {
                script {
                    fuzztargets.each { target ->
                        if(fileExists("${ARTIFACTS}/prev/${target}")) {
                            echo "Merging ${target} corpora"
                            sh "make merge SCC_FUZZ_TARGET=${target} SCC_FUZZ_CORPORA=${ARTIFACTS}/prev/${target}"
                        }
                        else {
                            echo "No corpora found for ${target}"
                        }
                    }
                }
            }
        }
        stage("Dynamic Fuzzing") {
            agent {
                docker {
                    image "${IMAGE}"
                }
            }
            steps {
                script {
                    echo "-- Running ${fuzzruns} iterations per target, each with duration ${fuzztime}s --"
                    echo "Expected duration per target: ${fuzztime * fuzzruns / 60}m, ${(fuzztime * fuzzruns) % 60}s"
                    fuzztargets.each { target ->
                        stage("Fuzz ${target}") {
                            for(int i = 0; i < fuzzruns; i++) {
                                echo "Fuzzing ${target} ${i + 1}/${fuzzruns}"
                                sh "make fuzz SCC_FUZZTIME=${fuzztime} SCC_FUZZ_TARGET=${target}"
                            }
                        }
                    }
                }
            }
        }
        stage("Gitlab Success") {
            steps {
                echo "-- Notifying Gitlab --"
                updateGitlabCommitStatus name: "build", state: "success"
            }
        }
    }
    post {
        always {
            node(null) {
                script {
                    if(!fileExists("${ARTIFACTS}/${CORPORA}")) {
                        new File("${ARTIFACTS}/${CORPORA}")
                    }

                    zip zipFile: "${ARTIFACTS}/${CORPORA}", archive:true, dir: "test/fuzz/corpora", overwrite: true
                    archiveArtifacts artifacts: "${ARTIFACTS}/${CORPORA}", fingerprint: true
                }
                echo "-- Removing dangling Docker images --"
                sh "docker system prune -f"

                echo "-- Cleaning up --"
                deleteDir()
            }
        }
    }
}
