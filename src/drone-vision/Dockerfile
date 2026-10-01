# Thin application image. The heavyweight AI/GStreamer dependencies live in
# drone-vision-base and are published through `make publish-vision-base`.
ARG VISION_BASE_IMAGE
FROM ${VISION_BASE_IMAGE}

WORKDIR /app
COPY scripts/    /app/scripts/
COPY entrypoint.sh /app/entrypoint.sh
RUN chmod +x /app/entrypoint.sh /app/scripts/*.py

ENTRYPOINT ["/app/entrypoint.sh"]
