ccs = ['gcc', 'clang']

pipeline {
    agent none
    environment {
        IMG='scc/build'
    }
    stages {
        stage('Gitlab Pending') {
            steps {
                echo 'Notifying Gitlab'
                updateGitlabCommitStatus name: 'build', state: 'pending'
            }
        }
        stage('Docker Image') {
            agent any
            steps {
                echo '-- Docker Image --'
                sh "docker build -f Dockerfile -t ${IMG} ."
            }
        }
        stage('Dynamic Build') {
            agent {
                docker {
                    image "${IMG}"
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
        stage('Dynamic Test') {
            agent {
                docker {
                    image "${IMG}"
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
        stage('Docs') {
            agent {
                docker {
                    image "${IMG}"
                }
            }
            steps {
                script {
                    sh "make doc"
                }
            }
        }
        stage('Gitlab Success') {
            steps {
                echo '-- Notifying Gitlab --'
                updateGitlabCommitStatus name: 'build', state: 'success'
            }
        }
    }
    post {
        always {
            node(null) {
                echo '-- Removing dangling Docker images --'
                sh 'docker system prune -f'

                echo '-- Cleaning up --'
                deleteDir()
            }
        }
    }
}
