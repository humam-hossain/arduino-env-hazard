from django.urls import path
from .views import get_data, post_data

urlpatterns = [
    path("get-data/", get_data),
    path("post-data/", post_data),
]
