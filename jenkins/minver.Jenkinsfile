def user_id
def user_name
def group_id
def group_name

def thread_count = 5

node('docker') {
    user_id = sh(returnStdout: true, script: 'id -u').trim()
    user_name = sh(returnStdout: true, script: 'id -un').trim()
    group_id = sh(returnStdout: true, script: 'id -g').trim()
    group_name = sh(returnStdout: true, script: 'id -gn').trim()
}

def wt_configure(Map args) {
    sh """/opt/cmake/bin/cmake .. \
            -DBUILD_EXAMPLES=ON \
            -DBUILD_TESTS=ON \
            -DCONNECTOR_FCGI=OFF \
            -DCONNECTOR_HTTP=ON \
            -DEXAMPLES_CONNECTOR=wthttp \
            -DENABLE_HARU=OFF \
            -DENABLE_PANGO=OFF \
            -DENABLE_POSTGRES=OFF \
            -DENABLE_QT4=OFF \
            -DENABLE_QT5=OFF \
            -DENABLE_SQLITE=ON \
            -DENABLE_SSL=OFF \
            -DHTTP_WITH_ZLIB=OFF \
            -DSHARED_LIBS=OFF \
            -DBOOST_PREFIX=/opt/boost \
            -DMULTI_THREADED=${args.mt}"""
}

pipeline {
    environment {
        EMAIL = credentials('wt-dev-mail')
    }
    options {
        buildDiscarder logRotator(numToKeepStr: '20')
        disableConcurrentBuilds()
    }
    agent {
        dockerfile {
            label 'docker'
            dir 'jenkins'
            filename 'minver.Dockerfile'
            additionalBuildArgs """--build-arg USER_ID=${user_id} \
                                   --build-arg USER_NAME=${user_name} \
                                   --build-arg GROUP_ID=${group_id} \
                                   --build-arg GROUP_NAME=${group_name} \
                                   --build-arg THREAD_COUNT=${thread_count}"""
        }
    }
    triggers {
        pollSCM('@daily')
    }
    stages {
        stage('Single-threaded') {
            steps {
                dir('build-st') {
                    wt_configure(mt: 'OFF')
                    sh "make -k -j${thread_count}"
                    sh "make -C examples -k -j${thread_count}"
                }
                dir('test') {
                    warnError('non-mt test.wt failed') {
                        sh "../build-st/test/test.wt"
                    }
                }
            }
        }
        stage('Multi-threaded') {
            steps {
                dir('build-mt') {
                    wt_configure(mt: 'ON')
                    sh "make -k -j${thread_count}"
                    sh "make -C examples -k -j${thread_count}"
                }
                dir('test') {
                    warnError('mt test.wt failed') {
                        sh "../build-mt/test/test.wt"
                    }
                }
            }
        }
    }
    post {
        cleanup {
            cleanWs()
        }
        regression {
            mail to: env.EMAIL,
                 subject: "Failed Pipeline: ${currentBuild.fullDisplayName}",
                 body: "Something is wrong with ${env.BUILD_URL}"
        }
        fixed {
            mail to: env.EMAIL,
                 subject: "Fixed Pipeline: ${currentBuild.fullDisplayName}",
                 body: "Build ${env.BUILD_URL} is OK"
        }
    }
}
