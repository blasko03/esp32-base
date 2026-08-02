FROM fedora:latest

RUN dnf -y update && \
    dnf -y install \
        python3 \
        python3-pip \
        systemd-udev

RUN cat > /etc/yum.repos.d/espressif-eim.repo <<'EOF'
[eim]
name=ESP-IDF Installation Manager
baseurl=https://dl.espressif.com/dl/eim/rpm/$basearch
enabled=1
gpgcheck=0
EOF

RUN dnf -y install eim-cli

ARG IDF_VERSION=v6.0.2

COPY .docker/expresif.toml /opt/espressif/expresif.toml
RUN eim install --config /opt/espressif/expresif.toml

RUN echo "source /opt/espressif/tools/activate_idf_$IDF_VERSION.sh" >> /etc/bashrc

CMD ["/bin/bash"]